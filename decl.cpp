#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

namespace c_compiler { namespace decl_impl {
  struct specifier {
    usr::flag_t m_flag;
    const type* m_type;
    static bool s_signed;
    static bool s_unsigned;
    static bool s_short;
    specifier() : m_flag(usr::NONE), m_type(0)
    {
      s_signed = false;
      s_unsigned = false;
      s_short = false;
    }
    static usr* s_usr;
    struct sweeper {
      sweeper(usr* u)
      {
        s_usr = u;
      }
      ~sweeper()
      {
        s_usr = 0;
      }
    };
  };
  bool specifier::s_signed;
  bool specifier::s_unsigned;
  bool specifier::s_short;
  usr* specifier::s_usr;
  bool comp_spec(parse::type_specifier*, parse::type_specifier*);
  specifier operator+(specifier, parse::type_specifier*);
  usr* install1(const specifier*, usr*, bool);
  usr* install2(usr*);
} } // end of namespace decl_impl and c_compiler

c_compiler::usr* c_compiler::declaration1(usr* u, bool ini)
{
  using namespace std;
  using namespace parse;
  using namespace decl_impl;
  assert(!decl_specs::s_stack.empty());
  decl_specs* ds = decl_specs::s_stack.top();
  sort(ds->begin(),ds->end(),comp_spec);
  specifier::sweeper sweepr2(u);
  specifier spec = accumulate(ds->begin(),ds->end(),specifier());
  decl_specs::s_curr.clear();
  if (u) {
    if (scope::current == &scope::root)
      u->m_type = u->m_type->vla2a();
    return install1(&spec,u,ini);
  }
  else {
    if ( !spec.m_type->get_tag() )
      error::decl::empty(parse::position);
    return 0;
  }
}

namespace c_compiler {
  int declaration3(usr*, parse::initializer*);
} // end of namespace c_compiler

void c_compiler::declaration2(usr* curr, parse::initializer* ini)
{
  using namespace std;
  auto_ptr<parse::initializer> sweeper(ini);
  int n = declaration3(curr,ini);
  const type* T = curr->m_type;
  typedef const array_type ARRAY;
  if ( T->m_id == type::ARRAY ){
    ARRAY* array = static_cast<ARRAY*>(T);
    if ( !array->dim() ){
      T = array->element_type();
      int m = T->size();
      if ( m )
        curr->m_type = array_type::create(T,(n + m - 1)/ m);
      else
        curr->m_type = array_type::create(T,1);
    }
  }
}

namespace c_compiler {
  void skipchk(const std::pair<int, var*>&);
} // end of namespace c_compiler

int c_compiler::declaration3(usr* curr, parse::initializer* ini)
{
  using namespace std;
  using namespace decl;
  if ( static_storage_duration(curr) ){
    with_initial* wi = static_cast<with_initial*>(curr);
    initializer::argument::dst = wi;
    initializer::argument arg(wi->m_type,wi->m_value,0,0,-1,-1,-1,-1);
    int n = initializer::eval(ini,arg);
#ifdef _DEBUG
    map<int,var*>& m = wi->m_value;
    typedef map<int,var*>::const_iterator IT;
    for ( IT p = m.begin() ; p != m.end() ; ++p ){
      var* v = p->second;
      if ( addrof* addrof = v->addrof_cast() ){
        static pvector<var> v;
        v.push_back(addrof);
      }
    }
#endif // _DEBUG
    if ( scope::current != &scope::root )
      expr::constant_flag = false;
    map<int, var*>& v = wi->m_value;
    for_each(v.begin(), v.end(), skipchk);
    return n;
  }
  else {
    initializer::argument::dst = curr;
    map<int,var*> v;
    initializer::argument arg(curr->m_type,v,0,0,-1,-1,-1,-1);
    int n = initializer::eval(ini,arg);
    if ( v.size() == 1 && curr->m_type->scalar() == v[0]->m_type->scalar() )
      code.push_back(new assign3ac(curr,v[0]));
    else
      for_each(v.begin(),v.end(),bind1st(ptr_fun(gen_loff),curr));
    return n;
  }
}

void c_compiler::skipchk(const std::pair<int, var*>& x)
{
  using namespace std;
  using namespace static_inline;
  var* v = x.second;
  addrof * a = v->addrof_cast();
  if (!a)
    return;
  var* r = a->m_ref;
  usr* u = r->usr_cast();
  addr3ac tmp(0, u);
  skip::chk_t arg(0);
  skip::check(&tmp, &arg);
}

namespace c_compiler { namespace decl_impl {
  void check_object(usr*);
} } // end of namespace decl_impl and c_compiler

namespace c_compiler {
  std::vector<FUNCS_ELEMENT_TYPE> funcs;
} // end of namespace c_compiler

void c_compiler::function_definition::begin(parse::decl_specs* ds, usr* u)
{
  using namespace std;
  using namespace decl_impl;
  using namespace parse;
  auto_ptr<decl_specs> sweeper(ds);
  vector<scope*>& children = scope::root.m_children;
  if (children.empty()) {
    using namespace error::decl;
    external::invalid(parse::position);
    string name = u->m_name;
    map<string, vector<usr*> >& usrs = scope::root.m_usrs;
    map<string, vector<usr*> >::iterator p = usrs.find(name);
    if ( p != usrs.end() ){
      const vector<usr*>& v = p->second;
      vector<usr*>::const_iterator q =
        find(v.begin(),v.end(),u);
      if ( q == v.end() )
        delete u;
    }
    else {
      vector<const type*> param;
      param.push_back(ellipsis_type::create());
      u->m_type = func_type::create(int_type::create(),param,true);
      usrs[name].push_back(u);
    }
    param_scope* tmp = new param_scope;
    tmp->m_parent = &scope::root;
    children.push_back(tmp);
    fundef::current = 0;
    return;
  }
  specifier spec;
  if (ds) {
    sort(ds->begin(),ds->end(),comp_spec);
    spec = accumulate(ds->begin(),ds->end(),spec);
    decl_specs::s_curr.clear();
  }
  else {
    using namespace error::decl;
    no_type(u);
    spec.m_type = int_type::create();
  }
  assert(children.size() == 1);
  param_scope* param = static_cast<param_scope*>(children.back());
  vector<usr*>& order = param->m_order;
  typedef const func_type FUNC;
  {
    typedef vector<usr*>::iterator IT;
    for ( IT p = order.begin() ; (p = find(p,order.end(),(usr*)0)) != order.end() ; ++p ){
      using namespace error::decl::declarator::func;
      int n = distance(order.begin(),p);
      parameter_omitted(parse::position,n+1);
      string name = new_name(".omit");
      FUNC* func = static_cast<FUNC*>(u->m_type);
      const vector<const type*>& parameter = func->param();
      const type* T = parameter[n];
      usr* u = new usr(name,T,usr::NONE,parse::position);
      *p = u;
      param->m_usrs[name].push_back(u);
    }
  }
  for_each(order.begin(),order.end(),check_object);
  {
    const type* T = u->m_type;
    FUNC* func = static_cast<FUNC*>(T);
    const vector<const type*>& parameter = func->param();
    T = parameter[0];
    if ( T->m_id == type::ELLIPSIS ){
      vector<const type*> param;
      param.push_back(void_type::create());
      u->m_type = func_type::create(func->return_type(),param,true);
    }
  }
  u = install1(&spec,u,false);
  fundef::current = new fundef(u,param);
  {
    const type* T = fundef::current->m_usr->m_type;
    FUNC* func = static_cast<FUNC*>(T);
    T = func->return_type();
    T = T->unqualified();
    if (T->m_id != type::VOID && !T->size()) {
      using namespace error::extdef::fundef;
      invalid_return(fundef::current->m_usr,T);
    }
  }
  usr::flag_t& flag = fundef::current->m_usr->m_flag;
  if (flag & usr::TYPEDEF) {
    using namespace error::extdef::fundef;
    typedefed(parse::position);
    flag = usr::flag_t(flag & ~usr::TYPEDEF);
  }
  string name = fundef::current->m_usr->m_name;
  map<string,usr*>::const_iterator p = function_definition::table.find(name);
  if (p != function_definition::table.end()) {
    using namespace error::extdef::fundef;
    multiple(parse::position,p->second);
  }
  else
    function_definition::table[name] = u;
}

void c_compiler::decl_impl::check_object(usr* u)
{
  const type* T = u->m_type;
  int size = T->size();
  if (!size) {
    using namespace error::decl;
    not_object(u,T);
    u->m_type = int_type::create();
  }
}

bool c_compiler::decl_impl::comp_spec(parse::type_specifier* x, parse::type_specifier* y)
{
  if (int a = x->first) {
    if (int b = y->first)
      return a < b;
    return false;
  }

  const type* a = x->second;
  if (const type* b = y->second)
    return a < b;
  return true;
}

namespace c_compiler { namespace decl_impl { namespace specifier_impl {
  struct table : std::map<int, void (*)(specifier*)> {
    table();
  } m_table;
} } } // end of namespace speicifier_impl, decl_impl and c_compiler


c_compiler::decl_impl::specifier
c_compiler::decl_impl::operator+(specifier spec, parse::type_specifier* p)
{
  using namespace specifier_impl;
  if ( int n = p->first ){
    table::const_iterator p = m_table.find(n);
    assert(p != m_table.end());
    p->second(&spec);
  }
  else {
    const type* T = p->second;
    if (spec.m_type) {
      using namespace error::decl;
      multiple_type(parse::position, spec.m_type, T);
      if (!T->size())
	T = spec.m_type;
    }
    spec.m_type = T;
    T = spec.m_type->unqualified();
    if ( T->m_id == type::FUNC )
      spec.m_flag = usr::flag_t(spec.m_flag | usr::FUNCTION);
    else if ( T->m_id == type::VARRAY )
      spec.m_flag = usr::flag_t(spec.m_flag | usr::VL);
  }
  return spec;
}

namespace c_compiler { namespace decl_impl { namespace specifier_impl {
  void void_handler(specifier*);
  void char_handler(specifier*);
  void int_handler(specifier*);
  void float_handler(specifier*);
  void double_handler(specifier*);
  void short_handler(specifier*);
  void long_handler(specifier*);
  void unsigned_handler(specifier*);
  void signed_handler(specifier*);
  void const_handler(specifier*);
  void volatile_handler(specifier*);
  void restrict_handler(specifier*);
  void typedef_handler(specifier*);
  void extern_handler(specifier*);
  void static_handler(specifier*);
  void auto_handler(specifier*);
  void register_handler(specifier*);
  void inline_handler(specifier*);
} } } // end of namespace specifier_impl, decl_impl and c_compiler

c_compiler::decl_impl::specifier_impl::table::table()
{
  (*this)[VOID_KW] = void_handler;
  (*this)[CHAR_KW] = char_handler;
  (*this)[INT_KW] = int_handler;
  (*this)[FLOAT_KW] = float_handler;
  (*this)[DOUBLE_KW] = double_handler;
  (*this)[SHORT_KW] = short_handler;
  (*this)[LONG_KW] = long_handler;
  (*this)[UNSIGNED_KW] = unsigned_handler;
  (*this)[SIGNED_KW] = signed_handler;
  (*this)[CONST_KW] = const_handler;
  (*this)[VOLATILE_KW] = volatile_handler;
  (*this)[RESTRICT_KW] = restrict_handler;
  (*this)[TYPEDEF_KW] = typedef_handler;
  (*this)[EXTERN_KW] = extern_handler;
  (*this)[STATIC_KW] = static_handler;
  (*this)[AUTO_KW] = auto_handler;
  (*this)[REGISTER_KW] = register_handler;
  (*this)[INLINE_KW] = inline_handler;
}

void c_compiler::decl_impl::specifier_impl::void_handler(specifier* spec)
{
  if ( spec->m_type ){
    using namespace error::decl;
    multiple_type(parse::position,spec->m_type,void_type::create());
  }
  spec->m_type = void_type::create();
}

void c_compiler::decl_impl::specifier_impl::char_handler(specifier* spec)
{
  if ( spec->m_type ){
    using namespace error::decl;
    multiple_type(parse::position,spec->m_type,char_type::create());
  }
  spec->m_type = char_type::create();
}

void c_compiler::decl_impl::specifier_impl::int_handler(specifier* spec)
{
  if ( spec->m_type ){
    using namespace error::decl;
    multiple_type(parse::position,spec->m_type,int_type::create());
  }
  spec->m_type = int_type::create();
}

void c_compiler::decl_impl::specifier_impl::float_handler(specifier* spec)
{
  if ( spec->m_type ){
    using namespace error::decl;
    multiple_type(parse::position,spec->m_type,float_type::create());
  }
  spec->m_type = float_type::create();
}

void c_compiler::decl_impl::specifier_impl::double_handler(specifier* spec)
{
  if ( spec->m_type ){
    using namespace error::decl;
    multiple_type(parse::position,spec->m_type,double_type::create());
  }
  spec->m_type = double_type::create();
}

namespace c_compiler { namespace decl_impl { namespace specifier_impl {
  namespace short_impl {
  struct table : std::map<const type*, const type*> {
    table();
  } m_table;
  } // end of namepsace short_impl
} } } // end of namepsace specifier_impl, decl_impl and c_compiler

void c_compiler::decl_impl::specifier_impl::short_handler(specifier* spec)
{
  specifier::s_short = true;
  short_impl::table::const_iterator p =
    short_impl::m_table.find(spec->m_type);
  if ( p != short_impl::m_table.end() )
    spec->m_type = p->second;
  else {
    using namespace error::decl;
    invalid_combination(parse::position,spec->m_type,"short");
  }
}

c_compiler::decl_impl::specifier_impl::short_impl::table::table()
{
  (*this)[0] = short_type::create();
  (*this)[int_type::create()] = short_type::create();
}

namespace c_compiler { namespace decl_impl { namespace specifier_impl {
  namespace long_impl {
  struct table : std::map<const type*, const type*> {
    table();
  } m_table;
  } // end of namepsace long_impl
} } } // end of namepsace specifier_impl, decl_impl and c_compiler

void c_compiler::decl_impl::specifier_impl::long_handler(specifier* spec)
{
  long_impl::table::const_iterator p =
    long_impl::m_table.find(spec->m_type);
  if ( p != long_impl::m_table.end() )
    spec->m_type = p->second;
  else {
    using namespace error::decl;
    invalid_combination(parse::position,spec->m_type,"long");
  }
}

c_compiler::decl_impl::specifier_impl::long_impl::table::table()
{
  (*this)[0] = long_type::create();
  (*this)[int_type::create()] = long_type::create();
  (*this)[long_type::create()] = long_long_type::create();
  (*this)[double_type::create()] = long_double_type::create();
}

namespace c_compiler { namespace decl_impl { namespace specifier_impl {
  namespace unsigned_impl {
  struct table : std::map<const type*, const type*> {
    table();
  } m_table;
  } // end of namespace unsigned_impl
} } } // end of namepsace specifier_impl, decl_impl and c_compiler

void c_compiler::decl_impl::specifier_impl::unsigned_handler(specifier* spec)
{
  using namespace error::decl;
  if ( specifier::s_signed )
    invalid_combination(parse::position,"signed","unsigned");
  if ( specifier::s_unsigned )
    invalid_combination(parse::position,"unsigned","unsigned");
  specifier::s_unsigned = true;
  unsigned_impl::table::const_iterator p =
    unsigned_impl::m_table.find(spec->m_type);
  if ( p != unsigned_impl::m_table.end() )
    spec->m_type = p->second;
  else
    invalid_combination(parse::position,spec->m_type,"unsigned");
}

c_compiler::decl_impl::specifier_impl::unsigned_impl::table::table()
{
  (*this)[0] = uint_type::create();
  (*this)[char_type::create()] = uchar_type::create();
  (*this)[short_type::create()] = ushort_type::create();
  (*this)[int_type::create()] = uint_type::create();
  (*this)[long_type::create()] = ulong_type::create();
  (*this)[long_long_type::create()] = ulong_long_type::create();
}

namespace c_compiler { namespace decl_impl { namespace specifier_impl {
  namespace signed_impl {
  struct table : std::map<const type*, const type*> {
    table();
  } m_table;
  } // end of namespace signed_impl
} } } // end of namepsace specifier_impl, decl_impl and c_compiler

void c_compiler::decl_impl::specifier_impl::signed_handler(specifier* spec)
{
  using namespace error::decl;
  if ( specifier::s_signed )
    invalid_combination(parse::position,"signed","signed");
  if ( specifier::s_unsigned )
    invalid_combination(parse::position,"unsigned","signed");
  specifier::s_signed = true;
  signed_impl::table::const_iterator p =
    signed_impl::m_table.find(spec->m_type);
  if ( p != signed_impl::m_table.end() )
    spec->m_type = p->second;
  else
    invalid_combination(parse::position,spec->m_type,"signed");
}

c_compiler::decl_impl::specifier_impl::signed_impl::table::table()
{
  (*this)[0] = int_type::create();
  (*this)[char_type::create()] = schar_type::create();
  (*this)[short_type::create()] = short_type::create();
  (*this)[int_type::create()] = int_type::create();
  (*this)[long_type::create()] = long_type::create();
  (*this)[long_long_type::create()] = long_long_type::create();
}

namespace c_compiler { namespace decl_impl { namespace specifier_impl {
  void qualifier_handler(specifier*, int);
} } } // end of namepsace specifier_impl, decl_impl and c_compiler

void c_compiler::decl_impl::specifier_impl::const_handler(specifier* spec)
{
  qualifier_handler(spec,1 << 0);
}

void c_compiler::decl_impl::specifier_impl::volatile_handler(specifier* spec)
{
  qualifier_handler(spec,1 << 1);
}

void c_compiler::decl_impl::specifier_impl::restrict_handler(specifier* spec)
{
  qualifier_handler(spec,1 << 2);
}

void c_compiler::decl_impl::specifier_impl::qualifier_handler(specifier* spec, int cvr)
{
  using namespace std;
  if ( !spec->m_type ){
    using namespace error::decl;
    specifier::s_usr ? no_type(specifier::s_usr) : no_type(parse::position);
    spec->m_type = int_type::create();
  }
  const type* T = spec->m_type;
  spec->m_type = T->qualified(cvr);
}

void c_compiler::decl_impl::specifier_impl::typedef_handler(specifier* spec)
{
  usr::flag_t& flag = spec->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if (flag & mask) {
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::TYPEDEF);
  }
  else
    flag = usr::flag_t(flag | usr::TYPEDEF);
}

void c_compiler::decl_impl::specifier_impl::extern_handler(specifier* spec)
{
  usr::flag_t& flag = spec->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::EXTERN);
  }
  else
    flag = usr::flag_t(flag | usr::EXTERN);
}

void c_compiler::decl_impl::specifier_impl::static_handler(specifier* spec)
{
  usr::flag_t& flag = spec->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if (flag & mask) {
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::STATIC);
  }
  else
    flag = usr::flag_t(flag | usr::STATIC);
}

void c_compiler::decl_impl::specifier_impl::auto_handler(specifier* spec)
{
  usr::flag_t& flag = spec->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::AUTO);
  }
  else
    flag = usr::flag_t(flag | usr::AUTO);
}

void c_compiler::decl_impl::specifier_impl::register_handler(specifier* spec)
{
  usr::flag_t& flag = spec->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::REGISTER);
  }
  else
    flag = usr::flag_t(flag | usr::REGISTER);
}

void c_compiler::decl_impl::specifier_impl::inline_handler(specifier* spec)
{
  spec->m_flag = usr::flag_t(spec->m_flag | usr::INLINE);
}

bool c_compiler::decl::static_storage_duration(const usr* u)
{
  return u && (u->m_scope == &scope::root || u->m_flag & usr::STATIC);
}

int c_compiler::decl::gen_loff(usr* x, std::pair<int,var*> p)
{
  int offset = p.first;
  var* y = integer::create(offset);
  var* z = p.second;
  code.push_back(new loff3ac(x,y,z));
  return 0;
}

namespace c_compiler { namespace decl_impl { namespace variable_length {
  void allocate(usr*);
} } } // end of namespace variable_length, decl_impl and c_compiler

c_compiler::usr*
c_compiler::decl_impl::install1(const specifier* spec, usr* curr, bool ini)
{
  using namespace std;
  curr->m_flag = spec->m_flag;
  const type* T = curr->m_type;
  assert(T->backpatch());
  if ( !spec->m_type ){
    using namespace error::decl;
    no_type(curr);
    curr->m_type = T->patch(int_type::create(),curr);
  }
  else
    curr->m_type = T->patch(spec->m_type,curr);
  T = curr->m_type;
  assert(!T->backpatch());
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  usr::flag_t& flag = curr->m_flag;
  if (b && T->variably_modified()) {
    if (flag & usr::EXTERN) {
      using namespace error::decl::declarator::vm;
      invalid_linkage(curr);
      flag = usr::flag_t(flag & ~usr::EXTERN);
    }
  }
  if ( stmt::for_decl ){
    usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC);
    if (flag & mask){
      using namespace error::stmt::_for;
      invalid_storage(curr);
      flag = usr::flag_t(flag & ~mask);
    }
  }
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::FUNCTION | usr::VL);
  if ( !(flag & mask) ){
    if ( b || scope::current == &scope::root ){
      typedef const array_type ARRAY;
      ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
      if ( !array || array->dim() || !ini )
        check_object(curr);
    }
  }
  else if ( flag & usr::EXTERN ){
    if ( ini ){
      if ( scope::current == &scope::root ){
        using namespace warning::decl::initializer;
        with_extern(curr);
      }
      else {
        using namespace error::decl::initializer;
        with_extern(curr);
      }
      flag = usr::flag_t(flag & ~usr::EXTERN);
    }
  }
  else if ( flag & usr::FUNCTION ){
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      if ( b ){
        using namespace error::decl::storage;
        invalid_function(curr);
        flag = usr::flag_t(flag & ~mask);
      }
    }
  }
  if ( flag & usr::VL ){
    variable_length::allocate(curr);
    if ( ini ){
      using namespace error::decl::declarator;
      varray::initializer(curr);
    }
  }
  if (b) {
    usr::flag_t mask = usr::flag_t(usr::STATIC | usr::EXTERN);
    if ( flag & mask ){
          usr::flag_t& fun_flag = fundef::current->m_usr->m_flag;
      if ( fun_flag & usr::INLINE ){
        using namespace error::decl::func_spec;
        static_storage(curr);
        fun_flag = usr::flag_t(fun_flag & ~usr::INLINE);
      }
    }
  }
  if (flag & usr::INLINE) {
    using namespace error::decl::func_spec;
    if ( !(flag & usr::FUNCTION) ){
      not_function(curr);
      flag = usr::flag_t(flag & ~usr::INLINE);
    }
    else if ( curr->m_name == "main" ){
      main(curr);
      flag = usr::flag_t(flag & ~usr::INLINE);
    }
  }
  if (b) {
    scope* param = b->m_parent;
    if ( param->m_parent == &scope::root ){
      const map<string, vector<usr*> >& usrs = param->m_usrs;
      map<string, vector<usr*> >::const_iterator p =
        usrs.find(curr->m_name);
      if ( p != usrs.end() ){
        using namespace error::decl;
        usr* prev = p->second.back();
        redeclaration(prev,curr,true);
      }
    }
  }
  else if (scope::current == &scope::root) {
    usr::flag_t mask = usr::flag_t(usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      using namespace error::extdef;
      invalid_storage(parse::position);
      flag = usr::flag_t(flag & ~mask);
    }
  }
  if (flag & usr::TYPEDEF) {
    type_def* tmp = new type_def(*curr);
    if (flag & usr::VL) {
      vector<usr*>& v = stmt::label::vm;
      typedef vector<usr*>::reverse_iterator IT;
      IT p = find(rbegin(v), rend(v), curr);
      assert(p != rend(v));
      *p = tmp;
    }
    delete curr;
    curr = tmp;
  }
  if (ini && decl::static_storage_duration(curr)) {
    with_initial* tmp = new with_initial(*curr);
    delete curr;
    curr = tmp;
    if ( scope::current != &scope::root )
      expr::constant_flag = true;
  }
  if (ini && parse::parameter::old_style) {
    using namespace error::extdef::fundef;
    invalid_initializer(curr);
  }
  return install2(curr);
}

void c_compiler::decl_impl::variable_length::allocate(usr* u)
{
  if (scope::current == &scope::root)
    return;
  assert(scope::current->m_parent != &scope::root);
  usr::flag_t& flag = u->m_flag;
  usr::flag_t mask = usr::flag_t(usr::STATIC | usr::EXTERN);
  if ( flag & mask ){
    using namespace error::decl::declarator::varray;
    invalid_storage(u);
    flag = usr::flag_t(flag & ~mask);
  }
  stmt::label::vm.push_back(u);
  if ( flag & usr::TYPEDEF )
    return;
  const type* T = u->m_type;
  var* size = T->vsize();
  code.push_back(new alloca3ac(u,size));
}

namespace c_compiler { namespace decl_impl {
  bool conflict(usr* prev, usr* curr);
} } // end of namespace decl_impl and c_compiler

c_compiler::usr* c_compiler::decl_impl::install2(usr* curr)
{
  using namespace std;
  map<string, vector<usr*> >& usrs = scope::current->m_usrs;
  string name = curr->m_name;
  if ( name == "__func__" ){
    using namespace error::expr::underscore_func;
    declared(parse::position);
  }
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if ( p != usrs.end() ){
    const vector<usr*>& v = p->second;
    usr* prev = v.back();
    assert(prev != curr);
    if (conflict(prev, curr)){
      using namespace error::decl;
      redeclaration(prev,curr,false);
    }
    else {
      curr->m_type = composite(prev->m_type, curr->m_type);
      usr::flag_t flag = curr->m_flag;
      if ((flag & usr::FUNCTION) && (flag & usr::EXTERN)){
        using namespace static_inline;
        using namespace skip;
        table_t::iterator r = table.find(prev);
        if (r != table.end()) {
          info_t* info = r->second;
          usr::flag_t& flag = info->m_fundef->m_usr->m_flag;
          flag = usr::flag_t(flag | usr::EXTERN);
          table.erase(r);
          gencode(info);
        }
      }
    }
  }
  usrs[name].push_back(curr);
  return curr;
}

namespace c_compiler { namespace decl_impl {
  bool conflict(usr::flag_t, usr::flag_t);
  inline bool conflict(const type* x, const type* y)
  {
    return !compatible(x, y);
  }
} } // end of namespace decl_impl and c_compiler

bool c_compiler::decl_impl::conflict(usr* x, usr* y)
{
  if ( conflict(x->m_flag,y->m_flag) )
      return true;

  if ( scope::current == &scope::root ){
    if ((x->m_flag & usr::WITH_INI) && (y->m_flag & usr::WITH_INI))
      return true;
  }
  return conflict(x->m_type,y->m_type);
}

// Reference ISO/IEC C9899 6.9.2 External object definitions. You can know that this implementation
// doesn't satisfy it. But I intentionally do this.
bool c_compiler::decl_impl::conflict(usr::flag_t x, usr::flag_t y)
{
  if ((x & usr::FUNCTION) != (y & usr::FUNCTION))
    return true;
  usr::flag_t xx = usr::flag_t(x & (usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER));
  usr::flag_t yy = usr::flag_t(y & (usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER));

  if (x & usr::FUNCTION) {
    if (scope::current == &scope::root) {
      if (xx == usr::TYPEDEF && yy == usr::NONE)
        return true;
      if (xx == usr::NONE && yy == usr::TYPEDEF)
        return true;
      if (xx == usr::NONE && yy == usr::STATIC)
        return true;
      if (xx == usr::STATIC && yy == usr::NONE)
        return true;
      if (xx == usr::STATIC && yy == usr::EXTERN)
        return true;
      if (xx == usr::EXTERN && yy == usr::STATIC)
        return true;
      if (xx == usr::TYPEDEF && yy == usr::TYPEDEF)
        return true;
      return false;
    }

    if (xx == usr::TYPEDEF && yy == usr::NONE)
      return true;
    if (xx == usr::NONE && yy == usr::TYPEDEF)
      return true;
    if (xx == usr::TYPEDEF && yy == usr::TYPEDEF)
      return true;
    if (xx == usr::STATIC && yy == usr::NONE)
      return true;
    if (xx == usr::STATIC && yy == usr::EXTERN)
      return true;
    if (xx == usr::EXTERN && yy == usr::STATIC)
      return true;
    return false;
  }

  // not function
  if (scope::current == &scope::root) {
    if (xx == usr::TYPEDEF && yy == usr::NONE)
      return true;
    if (xx == usr::NONE && yy == usr::TYPEDEF)
      return true;
    if (xx == usr::STATIC && yy == usr::NONE)
      return true;
    if (xx == usr::STATIC && yy == usr::EXTERN)
      return true;
    if (xx == usr::NONE && yy == usr::STATIC)
      return true;
    if (xx == usr::EXTERN && yy == usr::STATIC)
      return true;
    if (xx == usr::TYPEDEF && yy == usr::TYPEDEF)
      return true;
    return false;
  }

  // not external, not function
  if (xx == usr::NONE && yy == usr::NONE)
    return true;
  if (xx == usr::TYPEDEF && yy == usr::NONE)
    return true;
  if (xx == usr::NONE && yy == usr::TYPEDEF)
    return true;
  if (xx == usr::STATIC && yy == usr::NONE)
    return true;
  if (xx == usr::STATIC && yy == usr::EXTERN)
    return true;
  if (xx == usr::NONE && yy == usr::STATIC)
    return true;
  if (xx == usr::EXTERN && yy == usr::STATIC)
    return true;
  if (xx == usr::TYPEDEF && yy == usr::TYPEDEF)
    return true;
  return false;
}

const c_compiler::type*
c_compiler::decl::declarator::func(const type* T, parse::parameter_list* pl, usr* u)
{
  using namespace std;
  auto_ptr<parse::parameter_list> sweeper(pl);
  if ( pl )
    return T->patch(func_type::create(backpatch_type::create(),*pl,false),u);
  else {
    vector<const type*> param;
    param.push_back(ellipsis_type::create());
    return T->patch(func_type::create(backpatch_type::create(),param,true),u);
  }
}

namespace c_compiler { namespace decl { namespace declarator { namespace func_impl {
  using namespace std;
  struct old_style {
    string operator()(var* v){ return static_cast<usr*>(v)->m_name; }
    static vector<string> il;
  };
  vector<string> old_style::il;
} } } } // end of namespace func_impl, declarator, decl and c_compiler

const c_compiler::type*
c_compiler::decl::declarator::func(const type* T, parse::identifier_list* il, usr* u)
{
  using namespace std;
  using namespace func_impl;
  auto_ptr<parse::identifier_list> sweeper(il);
  // old_style::il.empty() is almost always true, but sometimes not.
  transform(il->begin(),il->end(),back_inserter(old_style::il),old_style());
  vector<const type*> param;
  param.push_back(ellipsis_type::create());
  return T->patch(func_type::create(backpatch_type::create(),param,true),u);
}

namespace c_compiler {
  namespace decl {
    namespace declarator {
      namespace array_impl {
        bool inblock(scope* ptr)
        {
          assert(ptr);
          switch (ptr->m_id) {
          case scope::NONE:
            return false;
          case scope::BLOCK:
            return true;
          default:
            assert(ptr->m_id == scope::PARAM);
            return inblock(ptr->m_parent);
          }
        }
        bool inblock_param(scope* ptr)
        {
          assert(ptr);
          if (ptr->m_id == scope::PARAM)
            return inblock(ptr->m_parent);
          else
            return false;
        }
        int csz;
      } // end of namespace array_impl
    } // end of namespace declarator
  } // end of namespace decl
} // end of namespace c_compiler

const c_compiler::type*
c_compiler::decl::declarator::array(const type* T, var* v, bool asterisc, usr* u)
{
  using namespace std;
  int dim = 0;
  const type* bt = backpatch_type::create();
  if (v) {
    v = v->rvalue();
    if ( !v->m_type->integer() ){
      using namespace error::decl::declarator::array;
      not_integer(parse::position,u);
      v = v->cast(int_type::create());
    }
    if ( !v->isconstant() ){
      if (scope::current != &scope::root) {
        var* tmp = new var(v->m_type);
        if (v->lvalue()) {
          if (scope::current->m_id == scope::BLOCK) {
            block* b = static_cast<block*>(scope::current);
            tmp->m_scope = b;
            b->m_vars.push_back(tmp);
          }
          else
            garbage.push_back(tmp);
          code.push_back(new assign3ac(tmp, v));
          v = tmp;
        }
        if (array_impl::inblock_param(scope::current)) {
          int n = array_impl::csz;
          for_each(code.begin()+n, code.end(), [](tac* p){ delete p; });
          code.resize(n);
          return T->patch( array_type::create(bt, 0), u);
        }
        return T->patch(varray_type::create(bt, v), u);
      }
      using namespace error::decl::declarator::vm;
      file_scope(u);
      v = integer::create(1);
    }
    dim = v->value();
    if ( dim <= 0 ){
      using namespace error::decl::declarator::array;
      not_positive(parse::position,u);
      dim = 1;
    }
  }
  else if (asterisc) {
    if (scope::current->m_id != scope::PARAM) {
      using namespace error::decl::declarator::array;
      asterisc_dimension(parse::position,u);
    }
  }
  return T->patch(array_type::create(bt, dim), u);
}

void c_compiler::parse::parameter::enter()
{
  using namespace std;
  vector<scope*>& children = scope::current->m_children;
  scope* param = new param_scope;
  param->m_parent = scope::current;
  children.push_back(param);
  scope::current = param;
}

void c_compiler::parse::parameter::leave()
{
  if ( scope::current == &scope::root )
    return;
  using namespace std;
  scope* org = scope::current;
  scope::current = scope::current->m_parent;
  vector<scope*>& children = scope::current->m_children;
  if (org->m_parent == &scope::root) {
    if (children.size() > 1) {
      /*
       * void (*f(int a))(float a)
       * {
       *   ...
       * }
       */
      assert(children.back() == org);
      children.pop_back();
      delete org;
    }
  }
  else {
    assert(children.back() == org);
    children.pop_back();
    delete org;
  }
}

void c_compiler::parse::parameter::enter2()
{
  using namespace std;
  assert(scope::current == &scope::root);
  vector<scope*>& children = scope::current->m_children;
  if ( !children.empty() ){
    assert(children.size() == 1);
    scope::current = children.back();
  }
}

bool c_compiler::parse::parameter::old_style;

std::vector<c_compiler::var*> c_compiler::garbage;

void c_compiler::destroy_temporary()
{
  using namespace std;
  vector<scope*>& children = scope::root.m_children;
  for (auto p : children)
    delete p;
  children.clear();
  for (auto p : garbage)
    delete p;
  garbage.clear();
  for (auto p : code)
    delete p;
  code.clear();
  error::headered = false;
  stmt::label::data.clear();
  if (!generator::last)
    type::destroy_tmp();
  vector<string>& v = decl::declarator::func_impl::old_style::il;
  if ( !v.empty() ){
    using namespace error::decl::declarator::func;
    invalid_identifier_list(parse::position);
    v.clear();
  }
  stmt::label::vm.clear();
  if ( cmdline::simple_medium )
    names::reset();
}

c_compiler::static_inline::info_t::~info_t()
{
  using namespace std;
  for (auto p : m_code)
    delete p;
  for (auto p : m_tmp)
    delete p;
  param_scope* param = m_fundef->m_param;
  delete param;
  delete m_fundef;
}

namespace c_compiler {
  using namespace std;
  namespace function_definition {
    void remember(fundef* fdef, vector<tac*>& vt);
  } // end of namespace function_definition
  namespace static_inline {
    skip::table_t skip::table;
    namespace defer {
      map<string, vector<ref_t> > refs;
      map<string, set<usr*> > callers;
      map<usr*, vector<int> > positions;
    } // end of namespace defer
  } // end of namespace static_inline
} // end of namespace c_compiler

void c_compiler::static_inline::gencode(info_t* info)
{
  using namespace function_definition;
  fundef* fdef = info->m_fundef;
  vector<tac*>& vc = info->m_code;
  skip::chk_t arg(fdef);
  for_each(vc.begin(), vc.end(), bind2nd(ptr_fun(skip::check), &arg));
  if (arg.m_wait_inline) {
    assert(skip::table.find(fdef->m_usr) == skip::table.end());
    skip::table[fdef->m_usr] = info;
    return;
  }
  scope* param = info->m_fundef->m_param;
  struct ss {
    scope* m_org;
    ss(scope* ptr) : m_org(0)
    {
      vector<scope*>& ch = scope::root.m_children;
      if (ch.empty())
        ch.push_back(ptr);
      else {
        assert(ch.size() == 1);
        m_org = ch[0];
        ch[0] = ptr;
      }
    }
    ~ss()
    {
      vector<scope*>& ch = scope::root.m_children;
      if (ch.size() == 1)
        ch.pop_back();
      else
        assert(generator::last && ch.empty());
      if (m_org)
        ch.push_back(m_org);
    }
  } ss(param);
  if (cmdline::output_medium) {
    if (cmdline::output_optinfo)
      cout << "\nAfter optimization\n";
    dump(fdef, vc);
  }
  if (!error::counter) {
    if (generator::generate) {
      generator::interface_t tmp = {
        &scope::root,
        fdef,
        &vc
      };
      generator::generate(&tmp);
    }
    else if (generator::last) {
      remember(fdef, vc);
      info = 0;
    }
  }
  delete info;
}

const c_compiler::type*
c_compiler::parse::parameter_declaration(decl_specs* ds, usr* u)
{
  using namespace std;
  using namespace decl_impl;
  auto_ptr<decl_specs> sweeper(ds);
  sort(ds->begin(),ds->end(),comp_spec);
  specifier spec = accumulate(ds->begin(),ds->end(),specifier());
  decl_specs::s_curr.clear();
  usr::flag_t& flag = spec.m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if ( flag & mask ){
    using namespace error::decl::declarator::func;
    invalid_storage(parse::position,u);
    flag = usr::flag_t(flag & ~mask);
  }
  const type* T = spec.m_type;
  if (u) {
    T = u->m_type;
    if (const type* ptr = T->ptr_gen())
      T = u->m_type = ptr;
    u = install1(&spec,u,false);
    T = u->m_type;
  }
  if (const type* ptr = T->ptr_gen()) {
    T = ptr;
    if (u)
      u->m_type = T;
  }

  const type* U = T->unqualified();
  if (U->m_id != type::VOID) {
    if (scope::current->m_parent == &scope::root) {
      param_scope* param = static_cast<param_scope*>(scope::current);
      param->m_order.push_back(u);
    }
  }
  return T;
}

const c_compiler::type*
c_compiler::parse::parameter_declaration(decl_specs* ds, const type* T)
{
  using namespace std;
  using namespace decl_impl;
  if ( scope::current->m_parent == &scope::root ){
    param_scope* param = static_cast<param_scope*>(scope::current);
    param->m_order.push_back(0);
  }
  auto_ptr<decl_specs> sweeper(ds);
  sort(ds->begin(),ds->end(),comp_spec);
  specifier spec = accumulate(ds->begin(),ds->end(),specifier());
  decl_specs::s_curr.clear();
  usr::flag_t& flag = spec.m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if ( flag & mask ){
    using namespace error::decl::declarator::func;
    invalid_storage(parse::position,0);
    flag = usr::flag_t(flag & ~mask);
  }
  T = T->patch(spec.m_type,0);
  if ( const type* ptr = T->ptr_gen() )
    T = ptr;
  return T;
}

namespace c_compiler {
  namespace stmt {
    using namespace std;
    inline void
    label_blame(const pair<string, vector<label::used_t> >& p)
    {
      using namespace error::stmt::label;
      string name = p.first;
      const vector<label::used_t>& v = p.second;
      for (auto& u : v)
        not_defined(name, u.m_file);
    }
  } // end of namespace stmt
} // end of namespace c_compiler

void
c_compiler::function_definition::action(fundef* fdef, std::vector<tac*>& vc)
{
  using namespace std;
  using namespace static_inline;
  map<string, vector<stmt::label::used_t> >& m = stmt::label::used;
  for (auto& p : m) stmt::label_blame(p);
  m.clear();
  if (!error::counter && cmdline::optimize_level >= 1)
    optimize::action(fdef, vc);
  usr::flag_t flag = fdef->m_usr->m_flag;
  usr::flag_t mask = usr::flag_t(usr::INLINE | usr::STATIC);
  if (flag & mask)
    return skip::add(fdef, vc, true);

  skip::chk_t arg(fdef);
  for_each(vc.begin(), vc.end(), bind2nd(ptr_fun(skip::check), &arg));
  if (arg.m_wait_inline)
    return skip::add(fdef, vc, false);

  if (cmdline::output_medium) {
    if (cmdline::output_optinfo)
      cout << "\nAfter optimization\n";
    dump(fdef, vc);
  }
  if (!error::counter) {
    if (generator::generate) {
      generator::interface_t tmp = {
        &scope::root,
        fdef,
        &vc
      };
      generator::generate(&tmp);
    }
    else if (generator::last) {
      remember(fdef, vc);
      fundef::current = 0;
    }
  }
  fdef->m_usr->m_type = fdef->m_usr->m_type->vla2a();
}

void
c_compiler::static_inline::skip::check(tac* ptac, chk_t* arg)
{
  using namespace std;
  ++arg->m_pos;
  var* y = ptac->y;
  if (!y)
    return;
  usr* u = y->usr_cast();
  if (!u)
    return;
  usr::flag_t flag = u->m_flag;
  if (!(flag & usr::FUNCTION))
    return;
  usr::flag_t mask = usr::flag_t(usr::STATIC | usr::INLINE);
  if (!(flag & mask))
    return;

  table_t::iterator it = table.find(u);
  if (it != table.end()) {
    info_t* info = it->second;
    table.erase(it);
    return gencode(info);
  }

  string name = u->m_name;
  it = find_if(table.begin(),table.end(),
       [name](const pair<usr*, info_t*>& p){ return p.first->m_name == name; });
  if (it != table.end()) {
    info_t* info = it->second;
    table.erase(it);
    return gencode(info);
  }

  const map<string, usr*>& fdt = function_definition::table;
  if (fdt.find(name) != fdt.end())
    return;

  using namespace defer;
  refs[name].push_back(ref_t(name, flag, u->m_file, ptac->m_file));
  if (ptac->m_id == tac::ADDR)
    return;
  assert(ptac->m_id == tac::CALL);
  if (!(flag & usr::INLINE))
    return;
  arg->m_wait_inline = true;
  usr* v = arg->m_fundef->m_usr;
  callers[name].insert(v);
  positions[v].push_back(arg->m_pos);
}

void
c_compiler::function_definition::dump(const fundef* fdef,
                                      const std::vector<tac*>& vc)
{
  using namespace std;
  cout << fdef->m_usr->m_name << ":\n";
  for (auto p : vc) {
    cout << '\t';
    tac_impl::dump(cout, p);
    cout << '\n';
  }
  cout << '\n';
  scope_impl::dump();
}

namespace c_compiler {
  namespace static_inline {
    using namespace std;
    using namespace defer;
    namespace skip {
      void after_substitute(usr* ucaller, pair<string, info_t*> pcallee)
      {
        map<usr*, vector<int> >::iterator p = positions.find(ucaller);
        assert(p != positions.end());
        vector<int>& vi = p->second;
        table_t::iterator q = table.find(ucaller);
        assert(q != table.end());
        info_t* caller = q->second;
        {
          vector<tac*>& vc = caller->m_code;
          vector<scope*>& ch = caller->m_fundef->m_param->m_children;
          assert(!ch.empty());
          struct sweeper {
            scope* m_org;
            sweeper(scope* org) : m_org(scope::current)
            { scope::current = org; }
            ~sweeper(){ scope::current = m_org; }
          } sweeper(ch[0]);
          int delta = 0;
          typedef vector<int>::iterator IT;
          for (IT r = vi.begin() ; r != vi.end() ; ) {
            *r += delta;
            int n = *r;
            tac* ptac = vc[n];
            assert(ptac->m_id == tac::CALL);
            var* y = ptac->y;
            usr* fn = y->usr_cast();
            assert(fn);
            string name = pcallee.first;
            info_t* callee = pcallee.second;
            if (fn->m_name == name) {
              int before = vc.size();
              if (!error::counter && !cmdline::no_inline_sub)
                substitute(vc, n, callee);
              int after = vc.size();
              delta += after - before;
              r = vi.erase(r);
            }
            else
              ++r;
          }
        }
        if (vi.empty()) {
          positions.erase(p);
          if (cmdline::optimize_level >= 1)
            optimize::action(caller->m_fundef, caller->m_code);
          usr::flag_t flag = ucaller->m_flag;
          if (!(flag & (usr::STATIC|usr::INLINE))) {
            table.erase(q);
            gencode(caller);
          }
        }
      }
      void add(fundef* fdef, vector<tac*>& vc, bool f)
      {
        usr* u = fdef->m_usr;
        u->m_type = u->m_type->vla2a();
        vector<const type*> vt;
        type::collect_tmp(vt);
        info_t* info = new info_t(fdef,vc,vt);
        table[u] = info;
        vector<scope*>& ch = scope::root.m_children;
        assert(ch.size() == 1 && ch[0] == fdef->m_param);
        ch.clear();
        vc.clear();
        fundef::current = 0;
        if (!f)
          return;

        string callee = u->m_name;
        map<string, vector<ref_t> >::iterator p = refs.find(callee);
        if (p == refs.end())
          return;

        refs.erase(p);
        map<string, set<usr*> >::iterator q = callers.find(callee);
        if (q == callers.end()) {
          table.erase(u);
          return gencode(info);
        }

        const set<usr*>& su = q->second;
        usr::flag_t flag = u->m_flag;
        assert(flag & usr::INLINE);
        for_each(begin(su), end(su),
                 bind2nd(ptr_fun(after_substitute),make_pair(callee,info)));
        callers.erase(q);
      }
    } // end of namespace skip
  } // end of namespace static_inline
} // end of namespace c_compiler

void
c_compiler::function_definition::remember(fundef* fdef, std::vector<tac*>& vc)
{
  funcs.push_back(make_pair(fdef, vc));
  scope::root.m_children.clear();
  vc.clear();
}

namespace c_compiler {
  bool internal_linkage(usr*);
  namespace tac_impl {
    void inline_code(var*);
  }
} // end of namespace c_compiler

c_compiler::tac::tac(id_t id, var* xx, var* yy, var* zz)
  : m_id(id), x(xx), y(yy), z(zz), m_file(parse::position)
{
  if ( fundef::current ){
    usr::flag_t flag = fundef::current->m_usr->m_flag;
    if ( (flag & usr::INLINE) && !(flag & usr::STATIC) ){
      if (x) tac_impl::inline_code(x);
      if (y) tac_impl::inline_code(y);
      if (z) tac_impl::inline_code(z);
    }
  }
}

bool c_compiler::internal_linkage(usr* u)
{
  using namespace std;
  if (u->m_scope != &scope::root)
    return false;
  usr::flag_t flag = u->m_flag;
  if (flag & usr::FUNCTION)
    return false;
  if (!(flag & usr::STATIC))
    return false;
  string name = u->m_name;
  if (name[name.length() - 1] == '"')
    return false;
  return true;
}

void c_compiler::tac_impl::inline_code(var* v)
{
  if (v) {
    if (usr* u = v->usr_cast()) {
      if (internal_linkage(u)) {
        error::decl::func_spec::internal_linkage(parse::position,u);
        usr::flag_t& flag = fundef::current->m_usr->m_flag;
        flag = usr::flag_t(flag & ~usr::INLINE);
      }
    }
  }
}

namespace c_compiler { namespace parse { namespace parameter {
  void old_style0();
  inline void move(var *v)
  {
    using namespace std;
    vector<var*>::reverse_iterator p = find(garbage.rbegin(),garbage.rend(),v);
    if (p != garbage.rend()) {
      garbage.erase(p.base()-1);
      c_compiler::block* b = static_cast<c_compiler::block*>(scope::current);
      v->m_scope = b;
      b->m_vars.push_back(v);
    }
  }
} } } // end of namespace parameter, parse and c_compiler

void c_compiler::parse::block::enter()
{
  using namespace std;
  bool top = scope::current == &scope::root;
  if (top) {
    vector<scope*>& ch = scope::current->m_children;  
    assert(ch.size() == 1);
    scope::current = ch.back();
    parameter::old_style0();
  }
  scope* tmp = new c_compiler::block;
  tmp->m_parent = scope::current;
  vector<scope*>& children = scope::current->m_children;
  children.push_back(tmp);
  scope::current = tmp;
  if (top) {
    for (auto p : code) {
      if (p->x) parameter::move(p->x);
      if (p->y) parameter::move(p->y);
      if (p->z) parameter::move(p->z);
    }
  }
}

void c_compiler::parse::block::leave()
{
  using namespace std;
  scope::current = scope::current->m_parent;
  if ( scope::current->m_parent == &scope::root )
    scope::current = &scope::root;
}

namespace c_compiler { namespace parse { namespace parameter {
  const type* old_styler1(std::string);
  void old_styler2(const std::pair<std::string, std::vector<usr*> >&);
} } } // end of namespace parameter, parse and c_compiler

void c_compiler::parse::parameter::old_style0()
{
  using namespace std;
  vector<string>& v = decl::declarator::func_impl::old_style::il;
  if (v.empty())
    return;
  vector<const type*> param;
  transform(v.begin(),v.end(),back_inserter(param),old_styler1);
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(fundef::current->m_usr->m_type);
  const type* T = func_type::create(func->return_type(),param,true);
  fundef::current->m_usr->m_type = T;
  map<string, vector<usr*> >& u = scope::current->m_usrs;
  for_each(u.begin(),u.end(),old_styler2);
  v.clear();
}

const c_compiler::type*
c_compiler::parse::parameter::old_styler1(std::string name)
{
  using namespace std;
  assert(scope::current->m_parent == &scope::root);
  param_scope* param = static_cast<param_scope*>(scope::current);
  map<string, vector<usr*> >& usrs = param->m_usrs;
  map<string, vector<usr*> >::const_iterator p = usrs.find(name);
  if (p == usrs.end()) {
    using namespace error::decl::declarator::func;
    not_declared(parse::position,name);
    return int_type::create();
  }

  const vector<usr*>& v = p->second;
  usr* u = v.back();
  usr::flag_t& flag = u->m_flag;
  usr::flag_t mask = usr::flag_t(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if (flag & mask) {
    using namespace error::extdef::fundef;
    invalid_storage(u);
    flag = usr::flag_t(flag & ~mask);
  }
  vector<usr*>& order = param->m_order;
  order.push_back(u);
  return u->m_type;
}

void c_compiler::parse::parameter::old_styler2(const std::pair<std::string, std::vector<usr*> >& e)
{
  using namespace std;
  string name = e.first;
  vector<string>& v = decl::declarator::func_impl::old_style::il;
  vector<string>::const_iterator p = find(v.begin(),v.end(),name);
  if (p == v.end()) {
    using namespace error::decl::declarator::func;
    usr* u = e.second.back();
    not_parameter(u);
  }
}

const c_compiler::type*
c_compiler::decl::declarator::pointer(const type* pointer, const type* T)
{
  return T->patch(pointer,0);
}

const c_compiler::type*
c_compiler::parse::pointer(type_qualifier_list* p)
{
  using namespace std;
  auto_ptr<type_qualifier_list> sweeper(p);
  const type* ret = pointer_type::create(backpatch_type::create()); 
  if (p) {
    if (find(p->begin(),p->end(),CONST_KW) != p->end())
      ret = const_type::create(ret);
    if (find(p->begin(),p->end(),VOLATILE_KW) != p->end())
      ret = volatile_type::create(ret);
    if (find(p->begin(),p->end(),RESTRICT_KW) != p->end())
      ret = restrict_type::create(ret);
  }
  return ret;
}

namespace c_compiler { namespace decl_impl {
  tag* create(tag::kind_t, std::string, const file_t&);
} } // end of namespace decl_impl and c_compiler

c_compiler::tag* c_compiler::parse::tag_begin(tag::kind_t kind, usr* u)
{
  using namespace std;
  using namespace decl_impl;
  if ( kind == tag::ENUM )
    enumerator::prev = integer::create(0);
  auto_ptr<usr> sweeper2(u);
  string name = u ? u->m_name : new_name(".tag");
  map<string, tag*>& tags = scope::current->m_tags;
  map<string, tag*>::const_iterator p = tags.find(name);
  if ( p == tags.end() )
    return create(kind,name,position);
  tag* prev = p->second;
  if ( prev->m_kind != kind ){
    using namespace error::decl;
    redeclaration(parse::position,prev->m_file.back(),name);
    return prev;
  }
  pair<const type*, const type*> types = prev->m_types;
  if ( types.second ){
    using namespace error::decl;
    redeclaration(parse::position,prev->m_file.back(),name);
    return prev;
  }
  prev->m_file.push_back(position);
  return prev;
}

c_compiler::tag* c_compiler::decl_impl::create(tag::kind_t kind, std::string name, const file_t& file)
{
  using namespace std;
  map<string, tag*>& tags = scope::current->m_tags;
  tag* T = new tag(kind,name,file);
  tags[name] = T;
  T->m_types.first = incomplete_tagged_type::create(T);
  return T;
}

std::string c_compiler::tag::keyword(kind_t k)
{
  switch ( k ){
  case STRUCT: return "struct";
  case UNION:  return "union";
  case ENUM:   return "enum";
  default: assert(0); return "";
  }
}

const c_compiler::type*
c_compiler::parse::struct_or_union_specifier(tag* T, struct_declaration_list* sdl)
{
  using namespace std;
  auto_ptr<struct_declaration_list> sweeper(sdl);
  vector<usr*> member(*sdl);
  if (const type* r = T->m_types.second) {
    for (auto p : member) delete p;
    return r;
  }
  return T->m_types.second = record_type::create(T,member);
}

namespace c_compiler { namespace decl_impl { namespace member {
  usr* conv(const type*, usr*);
} } } // end of member, namespace decl_impl and c_compiler

c_compiler::parse::struct_declaration_list*
c_compiler::parse::struct_declaration(decl_specs* ds,
                                      struct_declarator_list* sdl)
{
  using namespace std;
  using namespace decl_impl;
  using namespace member;
  auto_ptr<decl_specs> p(ds);
  sort(p->begin(),p->end(),comp_spec);
  specifier spec = accumulate(p->begin(),p->end(),specifier());
  decl_specs::s_curr.clear();
  struct_declaration_list* ret = new struct_declaration_list;
  auto_ptr<struct_declarator_list> q(sdl);
  if (q.get())
    transform(q->begin(),q->end(),back_inserter(*ret),bind1st(ptr_fun(conv),spec.m_type));
  return ret;
}

c_compiler::usr* c_compiler::decl_impl::member::conv(const type* T, usr* u)
{
  const type* X = u->m_type;
  u->m_type = X->patch(T,u);
  return u;
}

const c_compiler::type* c_compiler::parse::bit_field(const type* T, var* cexpr, usr* u)
{
  using namespace error::decl::struct_or_union::bit_field;
  cexpr = cexpr->rvalue();
  if ( !cexpr->m_type->integer() ){
    not_integer_bit(u);
    cexpr->m_type = int_type::create();
  }
  int bit = 1;
  if ( !cexpr->isconstant() )
    not_constant(u);
  else
    bit = cexpr->value();
  if ( bit < 0 ){
    negative(u);
    bit = 1;
  }
  if (T->m_id != type::BACKPATCH){
    not_integer_type(u);
    T = backpatch_type::create();
  }
  return T->patch(bit_field_type::create(bit,backpatch_type::create()),0);
}

const c_compiler::type* c_compiler::parse::tag_type(int key, c_compiler::tag* T)
{
  return T->m_types.second ? T->m_types.second : T->m_types.first;
}

const c_compiler::type* c_compiler::parse::enum_specifier(tag* T)
{
  using namespace std;
  return T->m_types.second = enum_type::create(T,enumerator::prev->m_type);
}

void c_compiler::parse::enumerator::action(usr* u, var* v)
{
  using namespace std;
  using namespace error::decl::_enum;
  auto_ptr<usr> sweeper(u);
  if (v && !v->isconstant()) {
    not_constant(u);
    v = 0;
  }
  if (v && !v->m_type->integer()) {
    not_integer(u);
    v = 0;
  }
  if (!v)
    v = prev;
  u->m_type = v->m_type;
  string name = u->m_name;
  u->m_flag = usr::ENUM_MEMBER;
  decl_impl::install2(new enum_member(*u,static_cast<usr*>(v)));
  v = expr::binary('+', v, integer::create(1));
  prev = static_cast<usr*>(v);
}

c_compiler::usr* c_compiler::parse::enumerator::prev;

const c_compiler::type*
c_compiler::parse::type_name(decl_specs* sql, const type* T)
{
  using namespace std;
  using namespace decl_impl;
  auto_ptr<decl_specs> sweeper(sql);
  sort(sql->begin(),sql->end(),comp_spec);
  specifier spec = accumulate(sql->begin(),sql->end(),specifier());
  decl_specs::s_curr.clear();
  if (!T)
    return spec.m_type;
  return T->patch(spec.m_type,0);
}
