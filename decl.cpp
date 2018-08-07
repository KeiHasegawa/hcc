#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

namespace c_compiler { namespace decl_impl {
  struct specifier {
    usr::flag m_flag;
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
  assert(!decl_specs::m_stack.empty());
  decl_specs* ds = decl_specs::m_stack.top();
  sort(ds->begin(),ds->end(),comp_spec);
  specifier::sweeper sweepr2(u);
  specifier spec = accumulate(ds->begin(),ds->end(),specifier());
  if ( u )
    return install1(&spec,u,ini);
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
	void check_skipped2(const std::pair<int, var*>&);
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
	for_each(v.begin(), v.end(), check_skipped2);
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

void c_compiler::check_skipped2(const std::pair<int, var*>& x)
{
	using namespace std;
	var* v = x.second;
	addrof * a = v->addrof_cast();
	if (!a)
		return;
	var* r = a->m_ref;
	usr* u = r->usr_cast();
	call3ac dummy(0, u);
	function_definition::static_inline::check_skipped(&dummy);
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
  if ( children.empty() ){
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
  if ( ds ){
    sort(ds->begin(),ds->end(),comp_spec);
    spec = accumulate(ds->begin(),ds->end(),spec);
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
  install1(&spec,u,false);
  fundef::current = new fundef(u,param);
  {
    const type* T = fundef::current->m_usr->m_type;
    FUNC* func = static_cast<FUNC*>(T);
    T = func->return_type();
    if ( !T->compatible(void_type::create()) && !T->size() ){
      using namespace error::extdef::fundef;
      invalid_return(fundef::current->m_usr,T);
    }
  }
  usr::flag& flag = fundef::current->m_usr->m_flag;
  if ( flag & usr::TYPEDEF ){
    using namespace error::extdef::fundef;
    typedefed(parse::position);
    flag = usr::flag(flag & ~usr::TYPEDEF);
  }
  string name = fundef::current->m_usr->m_name;
  map<string,usr*>::const_iterator p = function_definition::table.find(name);
  if ( p != function_definition::table.end() ){
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
  if ( !size ){
    using namespace error::decl;
    not_object(u,T);
    size = int_type::create()->size();
    u->m_type = int_type::create();
  }
}

bool c_compiler::decl_impl::comp_spec(parse::type_specifier* x, parse::type_specifier* y)
{
  if ( int a = x->first ){
    if ( int b = y->first )
      return a < b;
    else
      return false;
  }
  else
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
    spec.m_type = p->second;
    const type* T = spec.m_type->unqualified();
    if ( T->m_id == type::FUNC )
      spec.m_flag = usr::flag(spec.m_flag | usr::FUNCTION);
    else if ( T->m_id == type::VARRAY )
      spec.m_flag = usr::flag(spec.m_flag | usr::VL);
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
  usr::flag& flag = spec->m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::TYPEDEF);
  }
  else
    flag = usr::flag(flag | usr::TYPEDEF);
}

void c_compiler::decl_impl::specifier_impl::extern_handler(specifier* spec)
{
  usr::flag& flag = spec->m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::EXTERN);
  }
  else
    flag = usr::flag(flag | usr::EXTERN);
}

void c_compiler::decl_impl::specifier_impl::static_handler(specifier* spec)
{
  usr::flag& flag = spec->m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::STATIC);
  }
  else
    flag = usr::flag(flag | usr::STATIC);
}

void c_compiler::decl_impl::specifier_impl::auto_handler(specifier* spec)
{
  usr::flag& flag = spec->m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::AUTO);
  }
  else
    flag = usr::flag(flag | usr::AUTO);
}

void c_compiler::decl_impl::specifier_impl::register_handler(specifier* spec)
{
  usr::flag& flag = spec->m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER);
  if ( flag & mask ){
    using namespace error::decl::storage;
    multiple(parse::position,flag,usr::REGISTER);
  }
  else
    flag = usr::flag(flag | usr::REGISTER);
}

void c_compiler::decl_impl::specifier_impl::inline_handler(specifier* spec)
{
  spec->m_flag = usr::flag(spec->m_flag | usr::INLINE);
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
  if ( !spec->m_type ){
    using namespace error::decl;
    no_type(curr);
    curr->m_type = T->patch(int_type::create(),curr);
  }
  else
    curr->m_type = T->patch(spec->m_type,curr);
  T = curr->m_type;
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  usr::flag& flag = curr->m_flag;
  if ( b && T->temporary(true) ){
    if ( flag & usr::EXTERN ){
      using namespace error::decl::declarator::vm;
      invalid_linkage(curr);
      flag = usr::flag(curr->m_flag & ~usr::EXTERN);
    }
    T->decide();
  }
  if ( stmt::for_decl ){
    usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC);
    if ( curr->m_flag & mask ){
      using namespace error::stmt::_for;
      invalid_storage(curr);
      flag = usr::flag(curr->m_flag & ~mask);
    }
  }
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::FUNCTION | usr::VL);
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
      flag = usr::flag(flag & ~usr::EXTERN);
    }
  }
  else if ( flag & usr::FUNCTION ){
    usr::flag mask = usr::flag(usr::STATIC | usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      if ( b ){
        using namespace error::decl::storage;
        invalid_function(curr);
        flag = usr::flag(flag & ~mask);
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
  if ( b ){
    usr::flag mask = usr::flag(usr::STATIC | usr::EXTERN);
    if ( flag & mask ){
	  usr::flag& fun_flag = fundef::current->m_usr->m_flag;
      if ( fun_flag & usr::INLINE ){
        using namespace error::decl::func_spec;
        static_storage(curr);
        fun_flag = usr::flag(fun_flag & ~usr::INLINE);
      }
    }
  }
  if ( flag & usr::INLINE ){
    using namespace error::decl::func_spec;
    if ( !(flag & usr::FUNCTION) ){
      not_function(curr);
      flag = usr::flag(flag & ~usr::INLINE);
    }
    else if ( curr->m_name == "main" ){
      main(curr);
      flag = usr::flag(flag & ~usr::INLINE);
    }
  }
  if ( b ){
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
  else if ( scope::current == &scope::root ){
    usr::flag mask = usr::flag(usr::AUTO | usr::REGISTER);
    if ( flag & mask ){
      using namespace error::extdef;
      invalid_storage(parse::position);
      flag = usr::flag(flag & ~mask);
    }
  }

  if ( ini && decl::static_storage_duration(curr) ){
    with_initial* tmp = new with_initial(*curr);
    delete curr;
    curr = tmp;
    if ( scope::current != &scope::root )
      expr::constant_flag = true;
  }
  if ( ini && parse::parameter::old_style ){
    using namespace error::extdef::fundef;
    invalid_initializer(curr);
  }
  return install2(curr);
}

void c_compiler::decl_impl::variable_length::allocate(usr* u)
{
  if ( scope::current == &scope::root )
    return;
  assert(scope::current->m_parent != &scope::root);
  usr::flag& flag = u->m_flag;
  usr::flag mask = usr::flag(usr::STATIC | usr::EXTERN);
  if ( flag & mask ){
    using namespace error::decl::declarator::varray;
    invalid_storage(u);
    flag = usr::flag(flag & ~mask);
  }
  stmt::label::vm.push_back(u);
  if ( flag & usr::TYPEDEF )
    return;
  const type* T = u->m_type;
  var* size = T->vsize();
  code.push_back(new alloc3ac(u,size));
  block* b = static_cast<block*>(scope::current);
  b->m_dealloc.push_back(new dealloc3ac(u,size));
}

namespace c_compiler { namespace decl_impl {
  bool conflict1(usr*, usr*);
} } // end of namespace decl_impl and c_compiler

std::map<std::string,std::vector<c_compiler::usr*> > c_compiler::function_definition::Inline::decled;

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
	vector<usr*>::const_iterator q = find_if(v.begin(), v.end(), bind2nd(ptr_fun(conflict1),curr));
    if ( q != v.end() ){
      using namespace error::decl;
	  usr* prev = *q;
      redeclaration(prev,curr,false);
    }
	else {
		usr* prev = v.back();
		usr::flag flag = curr->m_flag;
 		if ((flag & usr::FUNCTION) && (flag & usr::EXTERN)){
			using namespace function_definition::static_inline;
			skipped_t::iterator r = skipped.find(prev);
			if (r != skipped.end()) {
				info* i = r->second;
				usr::flag& f = i->m_fundef->m_usr->m_flag;
				f = usr::flag(f | usr::EXTERN);
				skipped.erase(r);
				gencode(i);
			}
		}
	}
  }
  usr::flag flag = curr->m_flag;
  if ( (flag & usr::INLINE) && (flag & usr::FUNCTION) ){
    if ( c_compiler_text[0] == ';' )
      function_definition::Inline::decled[name].push_back(curr);
    else {
      typedef map<string, vector<usr*> >::iterator IT;
      IT p = function_definition::Inline::decled.find(name);
      if ( p != function_definition::Inline::decled.end() )
        function_definition::Inline::decled.erase(p);
    }
  }
  usrs[name].push_back(curr);
  return curr;
}

void c_compiler::function_definition::Inline::nodef(const std::pair<std::string, std::vector<usr*> >& p)
{
  using namespace std;
  const vector<usr*>& v = p.second;
  for_each(v.begin(),v.end(),error::decl::func_spec::no_definition);
}

namespace c_compiler { namespace decl_impl {
  bool conflict(usr::flag, usr::flag);
  bool conflict(const type* x, const type* y){ return !x->compatible(y); }
} } // end of namespace decl_impl and c_compiler

bool c_compiler::decl_impl::conflict1(usr* x, usr* y)
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
bool c_compiler::decl_impl::conflict(usr::flag x, usr::flag y)
{
	if ((x & usr::FUNCTION) != (y & usr::FUNCTION))
		return true;
	usr::flag xx = usr::flag(x & (usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER));
	usr::flag yy = usr::flag(y & (usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO | usr::REGISTER));

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

const c_compiler::type* c_compiler::decl::declarator::func(const type* T, parse::parameter_list* pl, usr* u)
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
  struct old_style {
    std::string operator()(var* v){ return static_cast<usr*>(v)->m_name; }
    static std::vector<std::string> il;
  };
  std::vector<std::string> old_style::il;
} } } } // end of namespace func_impl, declarator, decl and c_compiler

const c_compiler::type* c_compiler::decl::declarator::func(const type* T, parse::identifier_list* il, usr* u)
{
  using namespace std;
  using namespace func_impl;
  auto_ptr<parse::identifier_list> sweeper(il);
  assert(old_style::il.empty());
  transform(il->begin(),il->end(),back_inserter(old_style::il),old_style());
  vector<const type*> param;
  param.push_back(ellipsis_type::create());
  return T->patch(func_type::create(backpatch_type::create(),param,true),u);
}

const c_compiler::type* c_compiler::decl::declarator::array(const type* T, var* v, bool asterisc, usr* u)
{
  using namespace std;
  using namespace error::decl::declarator::array;
  int dim = 0;
  if ( v ){
    v = v->rvalue();
    if ( !v->m_type->integer() ){
      not_integer(parse::position,u);
      v = v->cast(int_type::create());
    }
    if ( !v->isconstant() ){
      if ( scope::current != &scope::root ){
        int x = array_impl::point;
        int y = code.size();
        if ( x == y || scope::current->m_id == scope::BLOCK )
          return array_impl::varray(T,v,u);
        else {
          vector<tac*> tmp;
          copy(code.begin()+x,code.begin()+y,back_inserter(tmp));
          code.resize(x);
          return array_impl::varray(T,v,tmp,u);
        }
      }
      using namespace error::decl::declarator::vm;
      file_scope(u);
      v = integer::create(1);
    }
    dim = v->value();
    if ( dim <= 0 ){
      not_positive(parse::position,u);
      dim = 1;
    }
  }
  else if ( asterisc ){
    if ( scope::current->m_id == scope::BLOCK || scope::current == &scope::root ){
      using namespace error::decl::declarator::array;
      asterisc_dimension(parse::position,u);
    }
  }
  return T->patch(array_type::create(backpatch_type::create(),dim),u);
}

const c_compiler::type* c_compiler::decl::declarator::array_impl::varray(const type* T, var* dim, usr* u)
{
  return T->patch(varray_type::create(backpatch_type::create(),dim),u);
}

const c_compiler::type* c_compiler::decl::declarator::array_impl::varray(const type* T, var* dim, const std::vector<tac*>& c, usr* u)
{
  return T->patch(varray_type::create(backpatch_type::create(),dim,c),u);
}

void c_compiler::parse::parameter::enter()
{
  using namespace std;
  vector<scope*>& children = scope::current->m_children;
  scope* param = new param_scope;
  param->m_parent = scope::current;
  children.push_back(param);
  scope::current = param;
  ++depth;
}

int c_compiler::parse::parameter::depth;

void c_compiler::parse::parameter::leave()
{
  if ( scope::current == &scope::root )
    return;
  struct sweeper {
    scope* m_scope;
    sweeper(scope* ptr)
    {
      using namespace std;
      using namespace c_compiler;
      if ( ptr->m_parent == &scope::root ){
        vector<scope*>& children = scope::root.m_children;
        if ( children.size() == 1 )
          m_scope = 0;
        else
          m_scope = ptr;
      }
      else
        m_scope = ptr;
    }
    ~sweeper()
    {
      using namespace std;
      using namespace c_compiler;
      if ( m_scope ){
        vector<scope*>& children = m_scope->m_parent->m_children;
        assert(children.back() == m_scope);
        children.pop_back();
        delete m_scope;
      }
    }
  } sweeper(scope::current);
  scope::current = scope::current->m_parent;
  --depth;
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
  for_each(children.begin(),children.end(),deleter<scope>());
  children.clear();
  for_each(garbage.begin(),garbage.end(),deleter<var>());
  garbage.clear();
  for_each(code.begin(),code.end(),deleter<tac>());
  code.clear();
  error::headered = false;
  stmt::label::data.clear();
  if (!generator::wrap)
	  type::destroy_temporary();
  vector<string>& v = decl::declarator::func_impl::old_style::il;
  if ( !v.empty() ){
    using namespace error::decl::declarator::func;
    invalid_identifier_list(parse::position);
    v.clear();
  }
  stmt::label::vm.clear();
#if 0
  if (!generator::wrap)
	  constant<void*>::destroy_temporary();
#endif
  if ( cmdline::simple_medium )
    names::reset();
}

c_compiler::function_definition::static_inline::info::~info()
{
  using namespace std;
  for_each(m_code.begin(),m_code.end(),deleter<tac>());
  param_scope* param = m_fundef->m_param;
  delete param;
  delete m_fundef;
}

c_compiler::function_definition::static_inline::skipped_t
c_compiler::function_definition::static_inline::skipped;

namespace c_compiler {
	namespace function_definition {
		extern void just_remember(std::vector<tac*>& v);
	}
} // end of namespace function_definition and c_compiler

void c_compiler::function_definition::static_inline::gencode(info* info)
{
	if (generator::wrap)
		return;
	scope* param = info->m_fundef->m_param;
	scope::root.m_children.push_back(param);
	function_definition::action(info->m_fundef, info->m_code, false);
	scope::root.m_children.pop_back();
}

void c_compiler::function_definition::static_inline::just_refed::nodef(const std::pair<std::string,just_refed::info*>& p)
{
  usr::flag flag = p.second->m_flag;
  if ( flag & usr::STATIC )
    error::extdef::fundef::nodef(parse::position,p.first,p.second->m_file);
}

const c_compiler::type* c_compiler::parse::parameter_declaration(decl_specs* ds, usr* u)
{
  using namespace std;
  using namespace decl_impl;
  auto_ptr<decl_specs> sweeper(ds);
  sort(ds->begin(),ds->end(),comp_spec);
  specifier spec = accumulate(ds->begin(),ds->end(),specifier());
  usr::flag& flag = spec.m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if ( flag & mask ){
    using namespace error::decl::declarator::func;
    invalid_storage(parse::position,u);
    flag = usr::flag(flag & ~mask);
  }
  const type* T = spec.m_type;
  if ( u ){
    T = u->m_type;
    if ( const type* ptr = T->ptr_gen() )
      T = u->m_type = ptr;
    u = install1(&spec,u,false);
    T = u->m_type;
  }

  if ( !T->compatible(void_type::create()) ){
    if ( scope::current->m_parent == &scope::root ){
      param_scope* param = static_cast<param_scope*>(scope::current);
      param->m_order.push_back(u);
    }
  }
  return T;
}

const c_compiler::type* c_compiler::parse::parameter_declaration(decl_specs* ds, const type* T)
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
  usr::flag& flag = spec.m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if ( flag & mask ){
    using namespace error::decl::declarator::func;
    invalid_storage(parse::position,0);
    flag = usr::flag(flag & ~mask);
  }
  T = T->patch(spec.m_type,0);
  if ( const type* ptr = T->ptr_gen() )
    T = ptr;
  return T;
}

namespace c_compiler { namespace stmt {
  void label_blame(const std::pair<std::string, std::vector<label::used_t> >&);
  int label_blame2(std::string, label::used_t);
} } // end of namespace stmt and c_compiler

c_compiler::function_definition::static_inline::just_refed::table_t
c_compiler::function_definition::static_inline::just_refed::table;

namespace c_compiler { namespace function_definition { namespace Inline { namespace remember {
  extern void action(std::vector<tac*>&);
} } } } // end of namespace remember, Inline, function_definition and c_compiler

namespace c_compiler { namespace function_definition { namespace static_inline {
  extern void remember(std::vector<tac*>&);
} } } // end of namespace static_inline, function_definition and c_compiler

void c_compiler::function_definition::action(fundef* fdef, std::vector<tac*>& vc, bool skip)
{
  using namespace std;
  if (skip){
	map<string, vector<stmt::label::used_t> >& m = stmt::label::used;
	for_each(m.begin(), m.end(), stmt::label_blame);
	m.clear();
#if 0
	if (!Inline::after::lists.empty())
	  return Inline::remember::action(vc);
#endif
	if (!error::counter)
	  optimize::action(fdef, vc);
	usr::flag flag = fdef->m_usr->m_flag;
	usr::flag mask = usr::flag(usr::INLINE | usr::STATIC);
	if (flag & mask){
	  string name = fdef->m_usr->m_name;
	  using namespace static_inline;
	  just_refed::table_t::iterator p = just_refed::table.find(name);
	  if (p != just_refed::table.end()) {
		  just_refed::info* info = p->second;
		  usr::flag prev = info->m_func->m_flag;
		  delete info;
		  just_refed::table.erase(p);
#if 0
		  if ((prev & mask) && !(prev & usr::EXTSTATIC))
			  return static_inline::remember(v);
#endif
	  }
	  else if (!generator::wrap)
		return static_inline::remember(vc);
	}
  }

  for_each(vc.begin(), vc.end(), static_inline::check_skipped);

  if ( cmdline::output_medium ){
    if ( cmdline::output_optinfo )
      cout << "\nAfter optimization\n";
    dump(fdef, vc);
  }
  if ( !error::counter ){
    if ( generator::generate ){
      generator::interface_t tmp = {
        &scope::root,
        fdef,
        &vc
      };
      generator::generate(&tmp);
    }
    else if (generator::wrap) {
	just_remember(vc);
    }
  }
}

namespace c_compiler { namespace function_definition { namespace static_inline { 
	bool cmp_name(std::pair<usr*, info*> p, std::string name)
	{
		return p.first->m_name == name;
	}
} } }  // end of namespace static_inline, function_definition and c_compiler

void c_compiler::function_definition::static_inline::check_skipped(tac* tac)
{
	var* y = tac->y;
	if (!y)
		return;
	usr* u = y->usr_cast();
	if (!u)
		return;
	usr::flag flag = u->m_flag;
	if (!(flag & usr::FUNCTION))
		return;
	usr::flag mask = usr::flag(usr::STATIC | usr::INLINE);
	if (!(flag & mask))
		return;

	skipped_t::iterator p = skipped.find(u);
	if (p != skipped.end()) {
		info* i = p->second;
		skipped.erase(p);
		return gencode(i);
	}
	
	using namespace std;
	string name = u->m_name;
	if ( function_definition::table.find(name) == function_definition::table.end() )
		just_refed::table[name] = new just_refed::info(tac->m_file, flag, fundef::current->m_usr);
	else {
		p = find_if(skipped.begin(), skipped.end(), bind2nd(ptr_fun(cmp_name), name));
		if (p != skipped.end()) {
			info* i = p->second;
			skipped.erase(p);
			return gencode(i);
		}
	}
}

void c_compiler::function_definition::dump(const fundef* fdef, const std::vector<tac*>& v)
{
  using namespace std;
  cout << fdef->m_usr->m_name << ":\n";
  typedef vector<tac*>::const_iterator IT;
  for ( IT p = v.begin() ; p != v.end() ; ++p ){
    cout << '\t';
    tac_impl::dump(cout,*p);
    cout << '\n';
  }
  cout << '\n';
  scope_impl::dump();
}

namespace c_compiler { namespace function_definition { namespace Inline { namespace remember { namespace update {
  extern void action(std::string);
} } } } } // end of namespace update, remember, Inline, function_definition and c_compiler

void c_compiler::function_definition::static_inline::remember(std::vector<tac*>& v)
{
  using namespace std;
  usr* u = fundef::current->m_usr;
  static_inline::skipped[u] = new static_inline::info(fundef::current,v);
  scope::root.m_children.clear();
  v.clear();
  usr::flag flag = u->m_flag;
  if ( flag & usr::INLINE )
    Inline::remember::update::action(u->m_name);
  fundef::current = 0;
}

void c_compiler::function_definition::just_remember(std::vector<tac*>& v)
{
	funcs.push_back(make_pair(fundef::current, v));
	scope::root.m_children.clear();
	v.clear();
	fundef::current = 0;
}

namespace c_compiler { namespace function_definition { namespace Inline { namespace remember { namespace update {
  extern int handler(std::string, std::vector<std::string>*);
} } } } } // end of namespace update, remember, Inline, function_definition and c_compiler

void c_compiler::function_definition::Inline::remember::update::action(std::string name)
{
  using namespace std;
  vector<string> vs;
  handler(name,&vs);
  for_each(vs.begin(),vs.end(),action);
}

namespace c_compiler { namespace function_definition { namespace Inline { namespace remember {
  struct info {
    fundef* m_fundef;
    std::vector<tac*> m_code;
    std::vector<after*> m_list;
    info(fundef* a, const std::vector<tac*>& b, const std::vector<after*>& c)
      : m_fundef(a), m_code(b), m_list(c) {}
    ~info();
  };
  std::list<info*> todo;
  extern bool finish(info*, std::string);
} } } } // end of namespace remember, Inline, function_definition and c_compiler

int c_compiler::function_definition::Inline::remember::update::handler(std::string in, std::vector<std::string>* vs)
{
  using namespace std;
  list<info*> tmp;
  while ( !todo.empty() ){
    Inline::remember::info* info = todo.front();
    todo.pop_front();
    string name = info->m_fundef->m_usr->m_name;
    usr::flag flag = info->m_fundef->m_usr->m_flag;
    if ( finish(info,in) ){
      if ( flag & usr::INLINE )
        vs->push_back(name);
    }
    else
      tmp.push_back(info);
  }
  todo = tmp;
  return 0;
}

namespace c_compiler { namespace function_definition { namespace Inline { namespace remember {
  std::string name(info* info){ return info->m_fundef->m_usr->m_name; }
} } } } // end of namespace remember, Inline, function_definition and c_compiler


bool c_compiler::function_definition::Inline::resolve::flag;

void c_compiler::function_definition::Inline::resolve::action()
{
  using namespace std;
  vector<string> vs;
  transform(remember::todo.begin(),remember::todo.end(),back_inserter(vs),remember::name);
  flag = true;
  vector<string> dummy;
  for_each(vs.begin(),vs.end(),bind2nd(ptr_fun(remember::update::handler),&dummy));
  for_each(garbage.begin(),garbage.end(),deleter<var>());
  garbage.clear();
}


void c_compiler::function_definition::Inline::remember::action(std::vector<tac*>& v)
{
  optimize::remember_action(v);
  todo.push_back(new info(fundef::current,v,after::lists));
  scope::root.m_children.clear();
  v.clear();
  fundef::current = 0;
  after::lists.clear();
}

bool c_compiler::function_definition::Inline::remember::finish(info* info, std::string name)
{
  using namespace std;
  vector<after*>& v = info->m_list;
  assert(!v.empty());
  typedef vector<after*>::iterator IT;
  for ( IT p = v.begin() ; p != v.end() ; ){
    if ( (*p)->expand(name,info->m_code) )
      p = v.erase(p);
    else
      ++p;
  }
  if ( !v.empty() )
    return false;
  param_scope* param = info->m_fundef->m_param;
  scope::root.m_children.push_back(param);
  function_definition::action(info->m_fundef,info->m_code,true);
  if ( !scope::root.m_children.empty() )
    scope::root.m_children.pop_back();
  delete info;
  return true;
}

c_compiler::function_definition::Inline::remember::info::~info()
{
  using namespace std;
  usr::flag flag = m_fundef->m_usr->m_flag;
  if ( !(flag & usr::INLINE) ){
    param_scope* param = m_fundef->m_param;
    delete param;
    delete m_fundef;
    for_each(m_code.begin(),m_code.end(),deleter<tac>());
  }
  fundef::current = 0;
}

namespace c_compiler {
  bool internal_linkage(usr*);
  namespace tac_impl {
    void inline_code(var*);
  }
} // end of namespace c_compiler

c_compiler::tac::tac(id_t ii, var* xx, var* yy, var* zz)
  : id(ii), x(xx), y(yy), z(zz), m_file(parse::position)
{
  if ( fundef::current ){
    usr::flag flag = fundef::current->m_usr->m_flag;
    if ( (flag & usr::INLINE) && !(flag & usr::STATIC) ){
      if ( x ) tac_impl::inline_code(x);
      if ( y ) tac_impl::inline_code(y);
      if ( z ) tac_impl::inline_code(z);
    }
  }
}

bool c_compiler::internal_linkage(usr* u)
{
	using namespace std;
	if (u->m_scope != &scope::root)
		return false;
	usr::flag f = u->m_flag;
	if (f & usr::FUNCTION)
		return false;
	if (!(f & usr::STATIC))
		return false;
	string name = u->m_name;
	if (name[name.length() - 1] == '"')
		return false;
	return true;
}

void c_compiler::tac_impl::inline_code(var* v)
{
  if ( v ){
    if ( usr* u = v->usr_cast() ){
      if ( internal_linkage(u) ){
        error::decl::func_spec::internal_linkage(parse::position,u);
        usr::flag& flag = fundef::current->m_usr->m_flag;
        flag = usr::flag(flag & ~usr::INLINE);
      }
    }
  }
}

void c_compiler::stmt::label_blame(const std::pair<std::string, std::vector<label::used_t> >& p)
{
  using namespace std;
  string name = p.first;
  const vector<label::used_t>& v = p.second;
  for_each(v.begin(),v.end(),bind1st(ptr_fun(label_blame2),name));
}

int c_compiler::stmt::label_blame2(std::string name, label::used_t u)
{
  const file_t& file = u.m_file;
  using namespace error::stmt::label;
  not_defined(name,file);
  return 0;
}

namespace c_compiler { namespace parse { namespace parameter {
  void old_style0();
  void decide(usr* u){ u->m_type->decide(); }
} } } // end of namespace parameter, parse and c_compiler

void c_compiler::parse::block::enter()
{
  using namespace std;
  bool top = scope::current == &scope::root;
  if ( top ){
    vector<scope*>& children = scope::current->m_children;
    assert(children.size() == 1);
    scope::current = children.back();
    parameter::old_style0();
  }
  vector<scope*>& children = scope::current->m_children;
  scope* tmp = new c_compiler::block;
  tmp->m_parent = scope::current;
  children.push_back(tmp);
  scope::current = tmp;
  if ( top ){
    param_scope* param = static_cast<param_scope*>(tmp->m_parent);
    vector<usr*>& order = param->m_order;
    for_each(order.begin(),order.end(),parameter::decide);
  }
}

void c_compiler::parse::block::leave()
{
  using namespace std;
  if (scope::current->m_id == scope::BLOCK) {
	  c_compiler::block* b = static_cast<c_compiler::block*>(scope::current);
	  vector<tac*>& v = b->m_dealloc;
	  copy(v.rbegin(), v.rend(), back_inserter(code));
  }
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
  if ( v.empty() )
    return;
  vector<const type*> param;
  transform(v.begin(),v.end(),back_inserter(param),old_styler1);
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(fundef::current->m_usr->m_type);
  fundef::current->m_usr->m_type = func_type::create(func->return_type(),param,true);
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
  if ( p == usrs.end() ){
    using namespace error::decl::declarator::func;
    not_declared(parse::position,name);
    return int_type::create();
  }

  const vector<usr*>& v = p->second;
  usr* u = v.back();
  usr::flag& flag = u->m_flag;
  usr::flag mask = usr::flag(usr::TYPEDEF | usr::EXTERN | usr::STATIC | usr::AUTO);
  if ( flag & mask ){
    using namespace error::extdef::fundef;
    invalid_storage(u);
    flag = usr::flag(flag & ~mask);
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
  if ( p == v.end() ){
    using namespace error::decl::declarator::func;
    usr* u = e.second.back();
    not_parameter(u);
  }
}

const c_compiler::type* c_compiler::decl::declarator::pointer(const type* pointer, const type* T)
{
  return T->patch(pointer,0);
}

const c_compiler::type* c_compiler::parse::pointer(type_qualifier_list* p)
{
  using namespace std;
  auto_ptr<type_qualifier_list> sweeper(p);
  const type* ret = pointer_type::create(backpatch_type::create()); 
  if ( p ){
    if ( find(p->begin(),p->end(),CONST_KW) != p->end() )
      ret = const_type::create(ret);
    if ( find(p->begin(),p->end(),VOLATILE_KW) != p->end() )
      ret = volatile_type::create(ret);
    if ( find(p->begin(),p->end(),RESTRICT_KW) != p->end() )
      ret = restrict_type::create(ret);
  }
  return ret;
}

namespace c_compiler { namespace decl_impl {
  tag* create(tag::kind, std::string, const file_t&);
} } // end of namespace decl_impl and c_compiler

c_compiler::tag* c_compiler::parse::tag_begin(tag::kind kind, usr* u)
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

c_compiler::tag* c_compiler::decl_impl::create(tag::kind kind, std::string name, const file_t& file)
{
  using namespace std;
  map<string, tag*>& tags = scope::current->m_tags;
  tag* T = new tag(kind,name,file);
  tags[name] = T;
  T->m_types.first = incomplete_tagged_type::create(T);
  return T;
}

std::string c_compiler::tag::keyword(kind k)
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
  if ( const type* r = T->m_types.second ){
    for_each(member.begin(),member.end(),deleter<usr>());
    return r;
  }
  return T->m_types.second = record_type::create(T,member);
}

namespace c_compiler { namespace decl_impl { namespace member {
  usr* conv(const type*, usr*);
} } } // end of member, namespace decl_impl and c_compiler

c_compiler::parse::struct_declaration_list* c_compiler::parse::struct_declaration(decl_specs* ds,
                                                                                  struct_declarator_list* sdl)
{
  using namespace std;
  using namespace decl_impl;
  using namespace member;
  auto_ptr<decl_specs> p(ds);
  sort(p->begin(),p->end(),comp_spec);
  specifier spec = accumulate(p->begin(),p->end(),specifier());
  struct_declaration_list* ret = new struct_declaration_list;
  auto_ptr<struct_declarator_list> q(sdl);
  if ( q.get() )
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
  if ( !T->backpatch() ){
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
  if ( v && !v->isconstant() ){
    not_constant(u);
    v = 0;
  }
  if ( v && !v->m_type->integer() ){
    not_integer(u);
    v = 0;
  }
  if ( !v )
    v = prev;
  u->m_type = v->m_type;
  string name = u->m_name;
  u->m_flag = usr::ENUM_MEMBER;
  decl_impl::install2(new enum_member(*u,static_cast<usr*>(v)));
  v = v->add(integer::create(1));
  prev = static_cast<usr*>(v);
}

c_compiler::usr* c_compiler::parse::enumerator::prev;

const c_compiler::type* c_compiler::parse::type_name(decl_specs* sql, const type* T)
{
  using namespace std;
  using namespace decl_impl;
  auto_ptr<decl_specs> sweeper(sql);
  sort(sql->begin(),sql->end(),comp_spec);
  specifier spec = accumulate(sql->begin(),sql->end(),specifier());
  if ( !T )
    return spec.m_type;
  return T->patch(spec.m_type,0);
}

int c_compiler::parse::guess(int kw)
{
  using namespace std;
  using namespace c_compiler::parse;
  vector<int>& v = decl_specs::m_curr;
  vector<int>::const_iterator p = find(v.begin(),v.end(),TAG_NAME_LEX);
  if ( p != v.end() ){
    switch ( kw ){
    case STRUCT_KW: unputer("struct"); break;
    case UNION_KW:  unputer("union");  break;
    case ENUM_KW:   unputer("enum");   break;
    }
    error::parse::missing(parse::position,';');
    return ';';
  }
  else
    return kw;
}
