#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

c_compiler::scope c_compiler::scope::root;

c_compiler::scope* c_compiler::scope::current = &c_compiler::scope::root;

c_compiler::scope::~scope()
{
  for (auto p : m_children)
    delete p;

  for (auto& u : m_usrs)
    for (auto p : u.second)
      delete p;

  for (auto& p : m_tags)
    p.second->m_scope = 0;
}

c_compiler::block::~block()
{
  for (auto p : m_vars)
    delete p;
}

namespace c_compiler { namespace parse { namespace identifier {
  namespace judge_impl {
    int prev;
    bool type_lex(int);
    int peek();
    namespace peek_impl {
      extern bool nest;
    }
    bool explicit_type(const type_specifier*);
    int new_identifier(std::string);
    int lookup(std::string, scope* = scope::current);
    int no_spec(std::string);
  } // end of namespace judge_impl
} } } // end of namespace identifier, parse and c_compiler

namespace c_compiler {
  namespace parse {
    using namespace std;
    vector<int> decl_specs::s_curr;
    vector<int> decl_specs::s_temp;
    stack<decl_specs*> decl_specs::s_stack;
  } // end of namespace parse
} // end of namespace c_compiler

int c_compiler::parse::identifier::judge(std::string name)
{
  using namespace std;
  using namespace judge_impl;
  if ( peek_impl::nest )
    return IDENTIFIER_LEX;
  if ( !decl_specs::s_curr.empty() ){
    vector<int>& v = decl_specs::s_curr;
    if ( find_if(v.rbegin(),v.rend(),type_lex) != v.rend() )
      return new_identifier(name);
    int n = v.back();
    if ( n == STRUCT_KW || n == UNION_KW || n == ENUM_KW ){
      if ( peek() == '{' )
        return new_identifier(name);
      v.push_back(TAG_NAME_LEX);
    }
    if (decl_specs::s_stack.empty() || !decl_specs::s_stack.top()) {
      if (int r = lookup(name)) {
        if (r == TYPEDEF_NAME_LEX || r == TAG_NAME_LEX)
          return r;
      }
    }
    return new_identifier(name);
  }
  else if ( !decl_specs::s_stack.empty() && decl_specs::s_stack.top() ){
    if (prev == '(' && scope::current->m_id == scope::PARAM ) {
      // guess abstract-declarator
      if (lookup(name) == TYPEDEF_NAME_LEX)
	return TYPEDEF_NAME_LEX;
    }
    const vector<type_specifier*>& v = *decl_specs::s_stack.top();
    if ( find_if(v.rbegin(),v.rend(),explicit_type) != v.rend() )
      return new_identifier(name);
    if ( int r = lookup(name) )
      return r;
    return new_identifier(name);
  }
  else
    return no_spec(name);
}

bool c_compiler::parse::identifier::judge_impl::type_lex(int n)
{
  return VOID_KW <= n && n <= TYPEDEF_NAME_LEX;
}

bool c_compiler::parse::identifier::judge_impl::explicit_type(const type_specifier* p)
{
  if ( int n = p->first )
    return VOID_KW <= n && n <= UNSIGNED_KW;
  else
    return true;
}

int c_compiler::parse::identifier::judge_impl::new_identifier(std::string name)
{
  c_compiler_lval.m_usr = new usr(name,backpatch_type::create(),usr::NONE,parse::position);
  return IDENTIFIER_LEX;
}

int c_compiler::parse::identifier::judge_impl::lookup(std::string name, scope* ptr)
{
  using namespace std;
  if ( prev != STRUCT_KW && prev != UNION_KW && prev != ENUM_KW ){
    const map<string, vector<usr*> >& usrs = ptr->m_usrs;
    map<string, vector<usr*> >::const_iterator p = usrs.find(name);
    if (p != usrs.end()) {
      const vector<usr*>& v = p->second;
      assert(!v.empty());
      usr* u = v.back();
      if (u->m_flag & usr::ENUM_MEMBER) {
        enum_member* p = static_cast<enum_member*>(u);
        c_compiler_lval.m_usr = p->m_value;
        return INTEGER_CONSTANT_LEX;
      }
      c_compiler_lval.m_usr = u;
      if (u->m_flag & usr::TYPEDEF) {
        type_def* t = static_cast<type_def*>(u);
        t->m_refed.push_back(parse::position);
        return TYPEDEF_NAME_LEX;
      }
      const type* T = u->m_type;
      if (const pointer_type* G = T->ptr_gen())
        garbage.push_back(c_compiler_lval.m_var = new genaddr(G,T,u,0));
      return IDENTIFIER_LEX;
    }
  }
  else {
    const map<string, tag*>& tags = ptr->m_tags;
    map<string, tag*>::const_iterator q = tags.find(name);
    if ( q != tags.end() ){
      tag* T = q->second;
      c_compiler_lval.m_tag = T;
      return TAG_NAME_LEX;
    }
  }
  if ( ptr->m_parent )
    return lookup(name,ptr->m_parent);
  return 0;
}

namespace c_compiler { namespace parse { namespace identifier { namespace judge_impl {
  struct func {
    std::map<int, var*>& m_value;
    func(std::map<int, var*>& v) : m_value(v) {}
    int operator()(int, char);
  };
} } } }

int c_compiler::parse::identifier::judge_impl::func::operator()(int n, char c)
{
  m_value[n] = integer::create(c);
  return n + 1;
}

int c_compiler::parse::identifier::judge_impl::no_spec(std::string name)
{
  using namespace std;
  if ( prev == '.' || prev == ARROW_MK || prev == GOTO_KW )
    return new_identifier(name);
  if ( int r = lookup(name) )
    return r;
  if ( name == "__func__" ){
    string s;
    if ( fundef::current )
      s = fundef::current->m_usr->m_name;
    else {
      using namespace error::expr::underscore_func;
      outside(parse::position);
    }
    const type* T = char_type::create();
    T = const_type::create(T);
    T = array_type::create(T,s.length()+1);
    with_initial* u = new with_initial(name,T,parse::position);
    map<int, var*>& v = u->m_value;
    accumulate(s.begin(),s.end(),0,func(v));
    v[s.length()] = integer::create(char(0));
    scope::current->m_usrs[name].push_back(u);
    const pointer_type* G = T->ptr_gen();
    c_compiler_lval.m_var = new genaddr(G,T,u,0);
    garbage.push_back(c_compiler_lval.m_var);
    return IDENTIFIER_LEX;
  }
  int c = peek();
  switch ( c ){
  case ')': // identifiler list of old style
  case ',': // identifiler list of old style
  case '(': // function call or function declaration without return value
  case ':': // label: statement
    return new_identifier(name);
  }
  error::undeclared(parse::position,name);
  usr* u = new usr(name,int_type::create(),usr::NONE,parse::position);
  scope::current->m_usrs[name].push_back(c_compiler_lval.m_usr = u);
  return IDENTIFIER_LEX;
}

namespace c_compiler { namespace parse { namespace identifier {
  namespace judge_impl { namespace peek_impl {
    bool nest;
    struct sweeper {
      std::string m_next;
      sweeper(std::string next) : m_next(next) {}
      ~sweeper()
      {
        unputer(m_next);
        nest = false;
      }
    };
  } } // end of namespace peek_impl and judge_impl
} } } // end of namespace identifier, parse and c_compiler

int c_compiler::parse::identifier::judge_impl::peek()
{
  peek_impl::nest = true;
  int next = c_compiler_lex();
  peek_impl::sweeper sweeper(c_compiler_text);
  return next;
}

namespace c_compiler { namespace integer {
  usr* new_obj(std::string);
} } // end of namespace integer and c_compiler

c_compiler::usr* c_compiler::integer::create(std::string name)
{
  using namespace std;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<std::string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = integer::new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

namespace c_compiler { namespace integer_impl {
  usr* int_(std::string, int, std::string);
  usr* uint_(std::string, unsigned int, std::string);
  usr* long_long_(std::string, __int64, std::string);
  usr* ulong_long_(std::string, unsigned __int64, std::string);
  int expressible_int(const char* name, char** end);
  unsigned int expressible_uint(const char* name, char** end);
} } // end of namespace integer_impl and c_compiler

c_compiler::usr* c_compiler::integer::new_obj(std::string name)
{
  using namespace std;
  errno = 0;
  char* end = 0;
  const char* tmp = name.c_str();
  int i = integer_impl::expressible_int(tmp,&end);
  if ( errno != ERANGE ){
    string suffix = end;
    return integer_impl::int_(name,i,suffix);
  }
  if ( name[0] == '0' || *end == 'u' || *end == 'U' ){
    errno = 0;
    unsigned int ui = integer_impl::expressible_uint(tmp,&end);
    if ( errno != ERANGE ){
      string suffix = end;
      return integer_impl::uint_(name,ui,suffix);
    }
  }
  errno = 0;
  __int64 ll = strtoll(name.c_str(),&end,0);
  if ( errno != ERANGE ){
    string suffix = end;
    return integer_impl::long_long_(name,ll,suffix);
  }
  errno = 0;
  if ( name[0] == '0' || *end == 'u' || *end == 'U' || name[0] == '-' ){
    unsigned __int64 ull = strtoull(name.c_str(),&end,0);
    if ( errno != ERANGE ){
      string suffix = end;
      return integer_impl::ulong_long_(name,ull,suffix);
    }
  }
  if ( name[0] == '0' || *end == 'u' || *end == 'U' || name[0] == '-' ){
    typedef unsigned __int64 X;
    const type* T = ulong_long_type::create();
    using namespace error::literal::integer;
    too_large(parse::position,name,T);
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = ll;
    return c;
  }
  else {
    typedef __int64 X;
    const type* T = long_long_type::create();
    using namespace error::literal::integer;
    too_large(parse::position,name,T);
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = ll;
    return c;
  }
}

int c_compiler::integer_impl::expressible_int(const char* name, char** end)
{
  long int x = strtol(name,end,0);  
  if (sizeof(long) == sizeof(int))
    return x;
  assert(sizeof(long) == sizeof(long long) && sizeof(int) == 4);
  if (x & (~0L << 31))
    errno = ERANGE;
  return x;
}

unsigned int c_compiler::integer_impl::expressible_uint(const char* name, char** end)
{
  unsigned long int x = strtoul(name,end,0);  
  if (sizeof(long) == sizeof(int))
    return x;
  assert(sizeof(long) == sizeof(long long));
  unsigned int y = x;
  if (x != y)
    errno = ERANGE;
  return y;
}

namespace c_compiler { namespace integer_impl {
  template<class C> usr* common_int(std::string name, C value, const type* T, std::string suffix)
  {
    if ( suffix.empty() ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "u" || suffix == "U" ){
      typedef unsigned int X;
      const type* T = uint_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "l" || suffix == "L" ){
      typedef long int X;
      const type* T = long_type::create();
      T = const_type::create(T);
      if (T->size() <= sizeof(X)) {
        constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
        c->m_value = value;
        return c;
      }
      typedef __int64 XX;
      assert(T->size() == sizeof(XX));
      constant<XX>* cc = new constant<XX>(name, T, usr::SUB_CONST_LONG, parse::position);
      cc->m_value = value;
      return cc;
    }
    else if ( suffix == "ul" || suffix == "uL" || suffix == "Ul" || suffix == "UL" ||
              suffix == "lu" || suffix == "lU" || suffix == "Lu" || suffix == "LU" ){
      typedef unsigned long int X;
      const type* T = ulong_type::create();
      T = const_type::create(T);
      if (T->size() <= sizeof(X)) {
        constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
        c->m_value = value;
        return c;
      }
      typedef unsigned __int64 XX;
      assert(T->size() == sizeof(XX));
      constant<XX>* cc = new constant<XX>(name, T, usr::SUB_CONST_LONG, parse::position);
      cc->m_value = value;
      return cc;
    }
    else if ( suffix == "ll" || suffix == "LL" ){
      typedef __int64 X;
      const type* T = long_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else {
      assert(suffix == "ull" || suffix == "uLL" || suffix == "Ull" || suffix == "ULL"
        ||   suffix == "llu" || suffix == "LLu" || suffix == "llU" || suffix == "LLU");
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
  }
} } // end of namespace integer_impl and c_compiler

c_compiler::usr* c_compiler::integer_impl::int_(std::string name, int value, std::string suffix)
{
  const type* T = int_type::create();
  T = const_type::create(T);
  return common_int(name,value,T,suffix);
}

c_compiler::usr* c_compiler::integer_impl::uint_(std::string name, unsigned int value, std::string suffix)
{
  const type* T = uint_type::create();
  T = const_type::create(T);
  return common_int(name,value,T,suffix);
}

namespace c_compiler { namespace integer_impl {
  template<class C> usr* common_long_long(std::string name, C value, const type* T, std::string suffix)
  {
    if ( suffix.empty() ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "u" || suffix == "U" ){
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "l" || suffix == "L" ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "ul" || suffix == "uL" || suffix == "Ul" || suffix == "UL" ||
              suffix == "lu" || suffix == "lU" || suffix == "Lu" || suffix == "LU" ){
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else if ( suffix == "ll" || suffix == "LL" ){
      constant<C>* c = new constant<C>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
    else {
      assert(suffix == "ull" || suffix == "uLL" || suffix == "Ull" || suffix == "ULL"
        ||   suffix == "llu" || suffix == "LLu" || suffix == "llU" || suffix == "LLU");
      typedef unsigned __int64 X;
      const type* T = ulong_long_type::create();
      T = const_type::create(T);
      constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
      c->m_value = value;
      return c;
    }
  }
} } // end of namespace integer_impl and c_compiler

c_compiler::usr* c_compiler::integer_impl::long_long_(std::string name, __int64 value, std::string suffix)
{
  const type* T = long_long_type::create();
  T = const_type::create(T);
  return common_long_long(name,value,T,suffix);
}

c_compiler::usr* c_compiler::integer_impl::ulong_long_(std::string name, unsigned __int64 value, std::string suffix)
{
  const type* T = ulong_long_type::create();
  T = const_type::create(T);
  return common_long_long(name,value,T,suffix);
}

namespace c_compiler { namespace integer {
    template<class T> usr* common(T v, const type* (*pf)())
    {
      using namespace std;
      string name = new_name(".integer");
      const type* type = (*pf)();
      type = const_type::create(type);
      constant<T>* c = new constant<T>(name,type,usr::NONE,parse::position);
      c->m_scope = &scope::root;
      c->m_value = v;
      map<string, vector<usr*> >& usrs = scope::root.m_usrs;
      usrs[name].push_back(c);
      return c;
    }
    template<class T> usr* common_wrapper(T v, const type* (*pf)())
    {
      using namespace std;
      static map<T, usr*> table;
      typename map<T, usr*>::const_iterator p = table.find(v);
      if (p != table.end())
        return p->second;
      return table[v] = common<T>(v,pf);
    }
} } // end of namespace integer and c_compiler

c_compiler::usr* c_compiler::integer::create(char v)
{
  return common_wrapper(v,(const type* (*)())char_type::create);
}

c_compiler::usr* c_compiler::integer::create(signed char v)
{
  return common_wrapper(v,(const type* (*)())schar_type::create);
}

c_compiler::usr* c_compiler::integer::create(unsigned char v)
{
  return common_wrapper(v,(const type* (*)())uchar_type::create);
}

c_compiler::usr* c_compiler::integer::create(short int v)
{
  return common_wrapper(v,(const type* (*)())short_type::create);
}

c_compiler::usr* c_compiler::integer::create(unsigned short int v)
{
  return common_wrapper(v,(const type* (*)())ushort_type::create);
}

c_compiler::usr* c_compiler::integer::create(int v)
{
  return common_wrapper(v,(const type* (*)())int_type::create);
}

c_compiler::usr* c_compiler::integer::create(unsigned int v)
{
  return common_wrapper(v,(const type* (*)())uint_type::create);
}

c_compiler::usr* c_compiler::integer::create(long int v)
{
  using namespace std;
  typedef long int T;
  const type* Tc = const_type::create(long_type::create());
  if (Tc->size() <= sizeof(T)) {
    static map<T, usr*> table;
    map<T, usr*>::const_iterator p = table.find(v);
    if (p != table.end())
      return p->second;
    return table[v] = common(v,(const type* (*)())long_type::create);
  }
  typedef __int64 T2;
  assert(Tc->size() == sizeof(T2));
  static map<T2, usr*> table;
  map<T2, usr*>::const_iterator p = table.find(v);
  if (p != table.end())
    return p->second;
  usr* u = create((T2)v);
  u->m_type = Tc;
  u->m_flag = usr::SUB_CONST_LONG;
  return table[v] = u;
}

c_compiler::usr* c_compiler::integer::create(unsigned long int v)
{
  using namespace std;  
  typedef unsigned long int T;
  const type* Tc = const_type::create(ulong_type::create());
  if (Tc->size() <= sizeof(T)) {
    static map<T, usr*> table;
    map<T, usr*>::const_iterator p = table.find(v);
    if (p != table.end())
      return p->second;
    return table[v] = common(v,(const type* (*)())ulong_type::create);
  }
  typedef unsigned __int64 T2;
  assert(Tc->size() == sizeof(T2));
  static map<T2, usr*> table;
  map<T2, usr*>::const_iterator p = table.find(v);
  if (p != table.end())
    return p->second;
  usr* u = create((T2)v);
  u->m_type = Tc;
  u->m_flag = usr::SUB_CONST_LONG;
  return table[v] = u;
}

c_compiler::usr* c_compiler::integer::create(__int64 v)
{
  return common_wrapper(v,(const type* (*)())long_long_type::create);
}

c_compiler::usr* c_compiler::integer::create(unsigned __int64 v)
{
  return common_wrapper(v,(const type* (*)())ulong_long_type::create);
}

namespace c_compiler { namespace character {
  usr* new_obj(std::string);
} } // end of namespace character and c_compiler

c_compiler::usr* c_compiler::character::create(std::string name)
{
  using namespace std;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<std::string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = character::new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

namespace c_compiler {
  namespace character_impl {
    using namespace std;  
    struct simple_escape : map<string, usr*> {
      bool m_initialized;
      simple_escape() : m_initialized(false) {}
      void helper(string, string, int);
      void initialize();
    } m_simple_escape;
    usr* escape(string);
    usr* wide(string);
    usr* universal(string);
    usr* normal(string);
  } // end of namespace character_impl
} // end of namespace c_compiler

c_compiler::usr* c_compiler::character::new_obj(std::string name)
{
  using namespace std;
  using namespace character_impl;
  m_simple_escape.initialize();
  simple_escape::iterator p = m_simple_escape.find(name);
  if ( p != m_simple_escape.end() ){
    usr* ret = p->second;
    m_simple_escape.erase(p);
    return ret;
  }
  else if ( usr* u = escape(name) )
    return u;
  else if ( usr* u = universal(name) )
    return u;
  else if ( usr* u = wide(name) )
    return u;
  else
    return normal(name);
}

void c_compiler::character_impl::simple_escape::initialize()
{
  if ( m_initialized )
    return;
  helper("'\\\''","L'\\\''",'\'');
  helper("'\\\"'","L'\\\"'",'\"');
  helper("'\\?'", "L'\\?'",'\?');
  helper("'\\\\'","L'\\\\'",'\\');
  helper("'\\a'", "L'\\a'",'\a');
  helper("'\\b'", "L'\\b'",'\b');
  helper("'\\f'", "L'\\f'",'\f');
  helper("'\\n'", "L'\\n'",'\n');
  helper("'\\r'", "L'\\r'",'\r');
  helper("'\\t'", "L'\\t'",'\t');
  helper("'\\v'", "L'\\v'",'\v');
  m_initialized = true;
}

void c_compiler::character_impl::simple_escape::helper(std::string x, std::string y, int z)
{
  const type* u = char_type::create();
  u = const_type::create(u);
  constant<char>* a = new constant<char>(x,u,usr::NONE,file_t());
  (*this)[x] = a;
  const type* T = generator::wchar::type;
  T = const_type::create(T);
  switch (generator::wchar::id) {
  case type::SHORT:
    {
      typedef short X;
      constant<X>* b = new constant<X>(y, T, usr::NONE, file_t());
      (*this)[y] = b;
      a->m_value = b->m_value = z;
      return;
    }
  case type::USHORT:
    {
      typedef unsigned short X;
      constant<X>* b = new constant<X>(y, T, usr::NONE, file_t());
      (*this)[y] = b;
      a->m_value = b->m_value = z;
      return;
    }
  case type::INT:
    {
      typedef int X;
      constant<X>* b = new constant<X>(y, T, usr::NONE, file_t());
      (*this)[y] = b;
      a->m_value = b->m_value = z;
      return;
    }
  case type::UINT:
    {
      typedef unsigned X;
      constant<X>* b = new constant<X>(y, T, usr::NONE, file_t());
      (*this)[y] = b;
      a->m_value = b->m_value = z;
      return;
    }
  case type::LONG:
    {
      typedef long X;
      constant<X>* b = new constant<X>(y, T, usr::NONE, file_t());
      (*this)[y] = b;
      a->m_value = b->m_value = z;
      return;
    }
  default:
    {
      assert(generator::wchar::id == type::LONG);
      typedef unsigned long X;
      constant<X>* b = new constant<X>(y, T, usr::NONE, file_t());
      (*this)[y] = b;
      a->m_value = b->m_value = z;
      return;
    }
  }
}

namespace c_compiler {
  namespace character_impl {
    usr* cnst(std::string name, int v);
  } // end of namespace character_impl
} // end of namespace c_compiler

c_compiler::usr* c_compiler::character_impl::escape(std::string name)
{
  using namespace std;
  string s = name;
  if ( s[0] == 'L' )
    s.erase(0,1);
  assert(s[0] == '\'');
  assert(s.length() > 1);
  if ( s[1] != '\\' )
    return 0;
  s.erase(0,2);
  bool x = false;
  if ( s[0] == 'x' ){
    s = s.substr(1,s.length()-1);
    x = true;
  }
  char* end;
  unsigned int n = x ? strtoul(s.c_str(),&end,16) : strtoul(s.c_str(),&end,8);
  if ( end == &s[0] )
    return 0;
  if ( *end != '\'' ){
    string t = "\'";
    t += end;
    usr* u = character::create(t);
    constant<char>* c = static_cast<constant<char>*>(u);
    n <<= 8;
    n |= (unsigned int)c->m_value;
  }
  if ( name[0] == 'L' )
    return character_impl::cnst(name, n);
  else {
    typedef char X;
    const type* T = char_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
    c->m_value = n;
    return c;
  }
}

c_compiler::usr* c_compiler::character_impl::universal(std::string name)
{
  if ( name[1] != '\\' )
    return 0;
  if ( name[2] != 'u' && name[2] != 'U' )
    return 0;
  unsigned int n = strtoul(&name[3],0,16);
  if ( name[2] == 'u' ){
    return character_impl::cnst(name, n);
    /*
    typedef wchar_typedef X;
    const type* T = literal::wchar_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = n;
    return c;
    */
  }
  else {
    typedef unsigned int X;
    const type* T = uint_type::create();
    T = const_type::create(T);
    constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
    c->m_value = n;
    return c;
  }
}



namespace c_compiler {
  namespace character_impl {
    namespace wide_impl {
      int value(std::string);
    } // end of namespace wide_impl
  } // end of namespace character_impl
} // end of namespace c_compiler

c_compiler::usr* c_compiler::character_impl::wide(std::string name)
{
  using namespace std;
  if (name[0] != 'L')
    return 0;
  int v = wide_impl::value(name);
  return cnst(name, v);
}

c_compiler::usr* c_compiler::character_impl::cnst(std::string name, int v)
{
  const type* T = generator::wchar::type;
  T = const_type::create(T);
  switch (generator::wchar::id) {
  case type::SHORT:
    {
      typedef short X;
      constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
      c->m_value = v;
      return c;
    }
  case type::USHORT:
    {
      typedef unsigned short X;
      constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
      c->m_value = v;
      return c;
    }
  case type::INT:
    {
      typedef int X;
      constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
      c->m_value = v;
      return c;
    }
  case type::UINT:
    {
      typedef unsigned X;
      constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
      c->m_value = v;
      return c;
    }
  case type::LONG:
    {
      typedef long X;
      constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
      c->m_value = v;
      return c;
    }
  default:
    {
      assert(generator::wchar::id == type::ULONG);
      typedef unsigned long X;
      constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
      c->m_value = v;
      return c;
    }
  }
}

namespace c_compiler {
  namespace character_impl {
    namespace wide_impl {
      bool jis(std::string);
    }
  }
} // end of namespace wide_impl, character_impl and c_compiler

int c_compiler::character_impl::wide_impl::value(std::string name)
{
  using namespace std;
  assert(name[0] == 'L');
  assert(name[1] == '\'');
  pair<unsigned char, unsigned char> v;
  if (name[4] == '\'') { // L' normal normal '
    v.first = name[2];
    v.second = name[3];
  }
  else if (name[3] == '\'') // L' normal '
    v.second = name[2];
  else if (name[2] != '\\') {
    if (jis(name)) {
      v.first = name[5];
      v.second = name[6];
    }
    else { // L' normal escape '
      v.first = name[2];
      name = '\'' + name.substr(3);
      usr* u = character::create(name);
      constant<char>* c = static_cast<constant<char>*>(u);
      v.second = c->m_value;
    }
  }
  else { // L' simple-escape something '
    string x = name.substr(1, 3);
    x += "'";
    usr* y = character::create(x);
    constant<char>* a = static_cast<constant<char>*>(y);
    v.first = a->m_value;
    string w = "'" + name.substr(4);
    usr* z = character::create(w);
    constant<char>* b = static_cast<constant<char>*>(z);
    v.second = b->m_value;
  }
  return v.first << 8 | v.second;
}

bool c_compiler::character_impl::wide_impl::jis(std::string name)
{
  assert(name[0] == 'L');
  assert(name[1] == '\'');
  if (name.length() == 11) {
    return name[2] == 0x1b
      && name[3] == 0x24
      && name[4] == 0x42
      && name[7] == 0x1b
      && name[8] == 0x28
      && name[9] == 0x42
      && name[10] == '\'';
  }
  else
    return false;
}

c_compiler::usr* c_compiler::character_impl::normal(std::string name)
{
  assert(name[0] == '\'');
  if (name[2] != '\'') {
    using namespace error::literal::character;
    invalid(parse::position, name, char_type::create());
  }
  typedef char X;
  const type* T = char_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name, T, usr::NONE, parse::position);
  c->m_value = name[1];
  return c;
}
