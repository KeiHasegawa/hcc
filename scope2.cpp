#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

namespace c_compiler { namespace floating {
    usr* new_obj(std::string);
    using namespace std;
    template<class T> map<T, constant<T>* > table;
} } // end of namespace character and c_compiler

c_compiler::usr* c_compiler::floating::create(std::string name)
{
  using namespace std;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  if ( p != usrs.end() )
    return p->second.back();
  usr* u = floating::new_obj(name);
  u->m_scope = &scope::root;
  usrs[name].push_back(u);
  return u;
}

c_compiler::usr* c_compiler::floating::create(float x)
{
  using namespace std;
  typedef float X;
  map<X, constant<X>*>::const_iterator p = table<X>.find(x);
  if ( p != table<X>.end() )
    return p->second;
  string name = new_name(".float");
  const type* T = float_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  table<X>[x] = c;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

c_compiler::usr* c_compiler::floating::create(double x)
{
  using namespace std;
  typedef double X;
  map<X, constant<X>*>::const_iterator p = table<X>.find(x);
  if ( p != table<X>.end() )
    return p->second;
  string name = new_name(".double");
  const type* T = double_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  table<X>[x] = c;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

c_compiler::usr* c_compiler::floating::create(long double x)
{
  using namespace std;
  typedef long double X;
  map<X, constant<X>*>::const_iterator p = table<X>.find(x);
  if ( p != table<X>.end() )
    return p->second;
  string name = new_name(".long double");
  const type* T = long_double_type::create();
  T = const_type::create(T);
  constant<X>* c = new constant<X>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->m_value = x;
  table<X>[x] = c;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

c_compiler::usr* c_compiler::floating::create(unsigned char* b)
{
  using namespace std;
  int sz = long_double_type::create()->size();
  string name = new_name(".long double");
  const type* T = long_double_type::create();
  T = const_type::create(T);
  constant<long double>* c = new constant<long double>(name,T,usr::NONE,parse::position);
  c->m_scope = &scope::root;
  c->b = b;
  map<string, vector<usr*> >& usrs = scope::root.m_usrs;
  usrs[name].push_back(c);
  return c;
}

namespace c_compiler { namespace floating_impl {
  template<class T> struct value {
    T dec(std::string);
    T hex(std::string);
    T operator()(std::string);
  };
} } // end of namespace floating_impl and c_compiler

c_compiler::usr* c_compiler::floating::new_obj(std::string name)
{
  using namespace std;
  char suffix = *name.rbegin();
  if ( suffix == 'f' || suffix == 'F' ){
    float x = floating_impl::value<float>()(name);
    const type* T = float_type::create();
    T = const_type::create(T);
    constant<float>* c = new constant<float>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    return c;
  }
  else if ( suffix == 'l' || suffix == 'L' ){
    long double x = floating_impl::value<long double>()(name);
    const type* T = long_double_type::create();
    T = const_type::create(T);
    constant<long double>* c = new constant<long double>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    if ( generator::long_double ){
      int sz = long_double_type::create()->size();
      c->b = new unsigned char[sz];
      (*generator::long_double->bit)(c->b,name.c_str());
    }
    return c;
  }
  else {
    double x = floating_impl::value<double>()(name);
    const type* T = double_type::create();
    T = const_type::create(T);
    constant<double>* c = new constant<double>(name,T,usr::NONE,parse::position);
    c->m_value = x;
    return c;
  }
}

template<class T> T c_compiler::floating_impl::value<T>::operator()(std::string name)
{
  return name[0] == '0' && (name[1] == 'x' || name[1] == 'X') ? hex(name) : dec(name);
}

template<class T> T c_compiler::floating_impl::value<T>::dec(std::string name)
{
  if ( sizeof(T) <= sizeof(double) )
    return strtod(name.c_str(),0);
  else
    return strtold(name.c_str(),0);
}

template<class T> T c_compiler::floating_impl::value<T>::hex(std::string name)
{
  // name = 0 [xX] alpha . beta [pP] [+-]? gamma
  assert(name[0] == '0');
  assert(name[1] == 'x' || name[1] == 'X');
  char* p = const_cast<char*>(name.c_str());
  p += 2;
  T res = 0;
  if ( name[2] != '.' ){
    __int64 alpha = strtoll(p,&p,16);
    res = alpha;
  }
  if ( *p == '.' ){
    ++p;
    if ( isxdigit(*p) ){
      char* q;
      __int64 beta = strtoll(p,&q,16);
      T t = beta;
      for ( ; p != q ; ++p )
        t /= 16;
      res += t;
    }
  }
  assert(*p == 'p' || *p == 'P');
  ++p;
  int gamma = strtol(p,0,0);
  while ( gamma ){
    if ( gamma < 0 ){
      res /= 2;
      ++gamma;
    }
    else {
      res *= 2;
      --gamma;
    }
  }
  return res;
}

namespace c_compiler {
  namespace pointer {
    template<> usr* create<void*>(const type* T, void* v)
    {
      using namespace std;
      typedef void* X;
      bool temp = T->tmp();
      typedef pair<const type*, X> KEY;
      KEY key(T, v);
      typedef map<KEY, constant<X>*> table_t;
      static table_t table;
      if (!temp) {
        table_t::const_iterator p = table.find(key);
        if (p != table.end())
          return p->second;
      }
      string name = new_name(".pointer");
      constant<X>* c = new constant<X>(name, T, usr::CONST_PTR, parse::position);
      if (!temp)
        table[key] = c;
      c->m_value = v;
      if (temp) {
        map<string, vector<usr*> >& usrs = scope::current->m_usrs;
        usrs[name].push_back(c);
      }
      else {
        c->m_scope = &scope::root;
        map<string, vector<usr*> >& usrs = scope::root.m_usrs;
        usrs[name].push_back(c);
      }
      return c;
    }
    template<> usr* create<__int64>(const type* T, __int64 v)
    {
      using namespace std;
      typedef __int64 X;
      bool temp = T->tmp();
      typedef pair<const type*,X> KEY;
      KEY key(T, v);
      typedef map<KEY, constant<X>*> table_t;
      static table_t table;
      if (!temp) {
        table_t::const_iterator p = table.find(key);
        if (p != table.end())
          return p->second;
      }
      string name = new_name(".pointer");
      constant<X>* c = new constant<X>(name, T, usr::CONST_PTR, parse::position);
      if (!temp)
        table[key] = c;
      c->m_value = v;
      if (temp) {
        map<string, vector<usr*> >& usrs = scope::current->m_usrs;
        usrs[name].push_back(c);
      }
      else {
        c->m_scope = &scope::root;
        map<string, vector<usr*> >& usrs = scope::root.m_usrs;
        usrs[name].push_back(c);
      }
      return c;
    }
  } // end of namespace pointer
} // end of namespace c_compiler

namespace c_compiler { namespace literal { namespace string_impl {
  usr* new_obj(std::string);
} } } // end of namespace literal and c_compiler

c_compiler::genaddr* c_compiler::literal::stringa(std::string name)
{
  using namespace std;
  map<std::string, vector<usr*> >& usrs = scope::root.m_usrs;
  map<std::string, vector<usr*> >::const_iterator p =
    usrs.find(name);
  usr* u;
  if ( p != usrs.end() )
    u = p->second.back();
  else {
    u = string_impl::new_obj(name);
    u->m_scope = &scope::root;
    usrs[name].push_back(u);
  }
  const type* T = u->m_type;
  const pointer_type* G = T->ptr_gen();
  genaddr* ret = new genaddr(G,T,u,0);
  garbage.push_back(ret);
  return ret;
}

namespace c_compiler { namespace literal { namespace string_impl {
  class calc {
    std::map<int,var*>& m_value;
    bool m_wide;
    bool m_escape;
    bool m_hex_mode;
    bool m_oct_mode;
    bool m_shiftjis_state;
    int m_jis_state;
    int m_euc_state;
    char m_prev;
    static inline bool shift_jis_first(int c)
    {
      return 129 <= c && c <= 159 || 224 <= c && c <= 239;
    }
  public:
    struct acc_t {
      unsigned int m_hex;
      unsigned int m_oct;
      acc_t() : m_hex(0), m_oct(0) {}
    };
  private:
    acc_t* m_acc;
  public:
    calc(std::map<int,var*>& value, bool wide, acc_t* acc)
      : m_value(value), m_wide(wide), m_escape(false), m_hex_mode(false), m_oct_mode(false),
        m_shiftjis_state(false), m_acc(acc), m_jis_state(0), m_euc_state(0), m_prev(0) {}
    int operator()(int n, int c);
  };
} } } // end of namespace string_impl, literal and c_compiler


c_compiler::usr* c_compiler::literal::string_impl::new_obj(std::string name)
{
  using namespace std;
  with_initial* ret = new with_initial(name,0,parse::position);
  map<int,var*>& value = ret->m_value;
  bool wide = name[0] == '"' ? false : true;
  if ( wide ){
    ret->m_type = generator::wchar::type;
    name = name.substr(2,name.length()-3);
  }
  else {
    ret->m_type = char_type::create();
    name = name.substr(1,name.length()-2);
  }
  calc::acc_t acc;
  int size = accumulate(name.begin(),name.end(),0,calc(value,wide,&acc));
  if ( acc.m_hex ){
    int offset = wide ? size * generator::wchar::type->size() : size;
    usr* u = character_impl::integer_create(wide, acc.m_hex);
    value.insert(make_pair(offset,u));
    ++size;
  }
  if ( acc.m_oct ){
    int offset = wide ? size * generator::wchar::type->size() : size;
    usr* u = character_impl::integer_create(wide, acc.m_oct);
    value.insert(make_pair(offset,u));
    ++size;
  }
  int offset = wide ? size * generator::wchar::type->size() : size;
  usr* u = character_impl::integer_create(wide, 0);
  value.insert(make_pair(offset,u));
  ret->m_type = array_type::create(ret->m_type,++size);
  optimize::mark(ret);
  return ret;
}

int c_compiler::literal::string_impl::calc::operator()(int n, int c)
{
  using namespace std;
  if (c == '\\') {
    if (!m_shiftjis_state && !m_escape) {
      m_escape = true;
      return n;
    }
  }
  if ( m_escape ){
    if ( c == 'x' && !m_hex_mode && !m_oct_mode ){
      m_hex_mode = true;
      return n;
    }
    else if ( isdigit(c) && !m_hex_mode && !m_oct_mode ){
      m_oct_mode = true;
      m_acc->m_oct = c - '0';
      return n;
    }
  }
  if ( m_hex_mode ){
    if ( isxdigit(c) ){
      m_acc->m_hex <<= 4;
      if ( isdigit(c) )
        m_acc->m_hex += c - '0';
      else if ( isupper(c) )
        m_acc->m_hex += c - 'A';
      else
        m_acc->m_hex += c - 'a';
      return n;
    }
    else {
      int offset = m_wide ? n * generator::wchar::type->size() : n;
      usr* u = character_impl::integer_create(m_wide, m_acc->m_hex);
      m_value.insert(make_pair(offset,u));
      ++n;
      m_acc->m_hex = 0;
      m_hex_mode = m_escape = false;
    }
  }
  if ( m_oct_mode ){
    if ( isdigit(c) && c != '9' ){
      m_acc->m_oct <<= 3;
      m_acc->m_oct += c - '0';
      return n;
    }
    else {
      int offset = m_wide ? n * generator::wchar::type->size() : n;
      usr* u = character_impl::integer_create(m_wide, m_acc->m_oct);
      m_value.insert(make_pair(offset,u));
      ++n;
      m_acc->m_oct = 0;
      m_oct_mode = m_escape = false;
    }
  }
  if ( m_wide ){
    c = (unsigned char)c;
    if (!m_shiftjis_state && shift_jis_first(c)){
      m_shiftjis_state = true;
      m_prev = c;
      return n;
    }
    if ( m_shiftjis_state ){
      assert(64 <= c && c <= 126 || 128 <= c && c <= 252);
      m_shiftjis_state = false;
      usr* u = character_impl::integer_create(true, m_prev, c);
      m_value.insert(make_pair(n * generator::wchar::type->size(),u));
      return n + 1;
    }
    if ( m_jis_state == 0 && c == 0x1b ){
      m_jis_state = 1;
      return n;
    }
    if ( m_jis_state == 1 && c == 0x24 ){
      m_jis_state = 2;
      return n;
    }
    if ( m_jis_state == 2 && c == 0x42 ){
      m_jis_state = 3;
      return n;
    }
    if ( m_jis_state == 3 ){
      if ( c == 0x1b ){
        m_jis_state = 5;
        return n;
      }
      m_prev = c;
      m_jis_state = 4;
      return n;
    }
    if ( m_jis_state == 4 ){
      m_jis_state = 3;
      usr* u = character_impl::integer_create(true, m_prev, c);
      m_value.insert(make_pair(n * generator::wchar::type->size(),u));
      return n + 1;
    }
    if ( m_jis_state == 5 && c == 0x28 ){
      m_jis_state = 6;
      return n;
    }
    if ( m_jis_state == 6 && c == 0x42 ){
      m_jis_state = 0;
      return n;
    }
    if ( m_euc_state == 0 && c == 0x8e ){
      m_euc_state = 1;
      return n;
    }
    if ( !m_shiftjis_state && m_euc_state == 0 && 0xa1 <= c && c <= 0xfe ){
      m_euc_state = 2;
      m_prev = c;
      return n;
    }
    if ( m_euc_state == 1 )
      m_euc_state = 0;
    if ( m_euc_state == 2 ){
      m_euc_state = 0;
      usr* u = character_impl::integer_create(true, m_prev, c);
      m_value.insert(make_pair(n * generator::wchar::type->size(),u));
      return n + 1;
    }
  }
  else {
    if (!m_shiftjis_state) {
      c = (unsigned char)c;
      if (shift_jis_first(c))
	m_shiftjis_state = true;
    }
    else
      m_shiftjis_state = false;
  }
  int offset = m_wide ? n * generator::wchar::type->size() : n;
  if ( m_escape ){
    ostringstream os;
    if ( m_wide )
      os << 'L';
    os << "'\\" << char(c) << "'";
    usr* u = character::create(os.str());
    m_value.insert(make_pair(offset,u));
    m_escape = false;
  }
  else {
    usr* u = character_impl::integer_create(m_wide, c);
    m_value.insert(make_pair(offset,u));
  }
  return n + 1;
}

c_compiler::var* c_compiler::parse::string_concatenation(var* x, var* y)
{
  using namespace std;
  genaddr* xx = x->genaddr_cast();
  with_initial* xxx = static_cast<with_initial*>(xx->m_ref);
  std::string s = xxx->m_name;
  std::string::size_type u = s[0] == 'L' ? 2 : 1;
  s = s.substr(u,s.length()-u-1);
  genaddr* yy = y->genaddr_cast();
  with_initial* yyy = static_cast<with_initial*>(yy->m_ref);
  std::string t = yyy->m_name;
  std::string::size_type v = t[0] == 'L' ? 2 : 1;
  t = t.substr(v,t.length()-v-1);
  ostringstream os;
  if ( u == 2 || v == 2 )
    os << 'L';
  os << '"' << s << t << '"';
  return literal::stringa(os.str());
}

std::string c_compiler::ucn::conv(std::string name)
{
  using namespace std;
  typedef string::size_type T;
  T end = string::npos;
  for ( T p = name.find('\\') ; p != end ; p = name.find('\\',p+1) ){
    int c = name[p+1];
    if ( c == 'u' ){
      string s = name.substr(p+2,4);
      int n = strtol(s.c_str(),0,16);
      name.replace(p,6,1,n);
    }
    else if ( c == 'U' ){
      string s = name.substr(p+2,4);
      int n = strtol(s.c_str(),0,16);
      string t = name.substr(p+6,4);
      int m = strtol(t.c_str(),0,16);
      name.replace(p,10,1,n << 16 | m);
    }
  }
  return name;
}

namespace c_compiler { namespace usr_impl {
  struct simple_escape : std::map<std::string, char> {
    simple_escape();
  } m_simple_escape;
} } // end of namesapce usr_impl and c_cmpiler

c_compiler::usr_impl::simple_escape::simple_escape()
{
  (*this)["'\\\''"] = '\'';
  (*this)["'\\\"'"] = '\"';
  (*this)["'\\?'"] = '\?';
  (*this)["'\\\\'"] = '\\';
  (*this)["'\\a'"] = '\a';
  (*this)["'\\b'"] = '\b';
  (*this)["'\\f'"] = '\f';
  (*this)["'\\n'"] = '\n';
  (*this)["'\\r'"] = '\r';
  (*this)["'\\t'"] = '\t';
  (*this)["'\\v'"] = '\v';
}

std::string c_compiler::usr::keyword(flag_t f)
{
  using namespace std;
  ostringstream os;
  if ( f & TYPEDEF )
    os << "typedef";
  else if ( f & EXTERN ){
    os << "extern";
    if ( f & INLINE )
      os << " inline";
  }
  else if ( f & STATIC ){
    os << "static";
    if ( f & INLINE )
      os << " inline";
  }
  else if ( f & AUTO )
    os << "auto";
  else if ( f & REGISTER )
    os << "register";
  else if ( f & INLINE )
    os << "inline";
  return os.str();
}

c_compiler::fundef* c_compiler::fundef::current;

std::map<std::string, c_compiler::usr*> c_compiler::function_definition::table;
