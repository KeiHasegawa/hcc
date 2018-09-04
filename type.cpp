#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"

namespace c_compiler { namespace type_impl {
    struct sizeof_table : std::map<const type*, int> {
      sizeof_table();
    } sizeof_table;
    int m_pointer_sizeof = sizeof(void*);
} } // end of namespace type_impl and c_compiler

c_compiler::type_impl::sizeof_table::sizeof_table()
{
  (*this)[short_type::create()] = (*this)[ushort_type::create()] = sizeof(short);
  (*this)[int_type::create()] = (*this)[uint_type::create()] = sizeof(int);
  (*this)[long_type::create()] = (*this)[ulong_type::create()] = sizeof(long);
  (*this)[long_long_type::create()] = (*this)[ulong_long_type::create()] =
    sizeof(long long);
  (*this)[float_type::create()] = sizeof(float);
  (*this)[double_type::create()] = sizeof(double);
  (*this)[long_double_type::create()] = sizeof(long double);
}

void c_compiler::type_impl::update(int (*size)(const type*))
{
  if (!size)
        return;
  sizeof_table[short_type::create()] = sizeof_table[ushort_type::create()] = size(short_type::create());
  sizeof_table[int_type::create()] = sizeof_table[uint_type::create()] = size(int_type::create());
  sizeof_table[long_type::create()] = sizeof_table[ulong_type::create()] = size(long_type::create());
  sizeof_table[long_long_type::create()] = sizeof_table[ulong_long_type::create()] = size(long_long_type::create());
  sizeof_table[float_type::create()] = size(float_type::create());
  sizeof_table[double_type::create()] = size(double_type::create());
  sizeof_table[long_double_type::create()] = size(long_double_type::create());
  m_pointer_sizeof = size(pointer_type::create(void_type::create()));
}

bool c_compiler::type::compatible(const type* that) const
{
  return this == that->unqualified();
}

const c_compiler::type* c_compiler::type::composite(const type* that) const
{
  int cvr = 0;
  return this == that->unqualified(&cvr) ? qualified(cvr) : 0;
}

bool c_compiler::type::include_cvr(const type* that) const
{
  int cvr = 0;
  that->unqualified(&cvr);
  return !cvr;
}

std::pair<int, const c_compiler::type*> c_compiler::type::current(int nth) const
{
  using namespace std;
  return nth ? make_pair(-1,static_cast<const type*>(0)) : make_pair(0,this);
}

const c_compiler::type* c_compiler::type::qualified(int cvr) const
{
  const type* T = this;
  if ( cvr & 1 )
    T = const_type::create(T);
  if ( cvr & 2 )
    T = volatile_type::create(T);
  if ( cvr & 4 ){
    if ( T->m_id == type::POINTER )
      T = restrict_type::create(T);
    else {
      using namespace error::decl::qualifier;
      invalid(parse::position,T);
    }
  }
  return T;
}

namespace c_compiler { namespace type_impl {
  std::vector<const type*> temp1;
#ifdef _DEBUG
  pvector<const type> temp2;
#endif // _DEBUG
} } // end of namespace type_impl and c_compiler

int c_compiler::type::align() const
{
        switch (size()) {
        case 1: return 1;
        case 2: return 2;
        case 3:
        case 4: return 4;
        case 5: case 6: case 7: case 8: return 8;
        case 9: case 10: case 11: case 12: case 13:
        case 14: case 15: case 16:
        default: return 16;
        }
}

void c_compiler::type::destroy_temporary()
{
  using namespace std;
  vector<const type*>& v = type_impl::temp1;
  for_each(v.begin(),v.end(),deleter<const type>());
  v.clear();
}

c_compiler::void_type c_compiler::void_type::obj;

void c_compiler::void_type::decl(std::ostream& os, std::string name) const
{
  os << "void";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::char_type c_compiler::char_type::obj;

void c_compiler::char_type::decl(std::ostream& os, std::string name) const
{
  os << "char";
  if ( !name.empty() )
    os << ' ' << name;
}

const c_compiler::type* c_compiler::char_type::promotion() const
{
  return int_type::create();
}

c_compiler::schar_type c_compiler::schar_type::obj;

void c_compiler::schar_type::decl(std::ostream& os, std::string name) const
{
  os << "signed char";
  if ( !name.empty() )
    os << ' ' << name;
}

const c_compiler::type* c_compiler::schar_type::promotion() const
{
  return int_type::create();
}

c_compiler::uchar_type c_compiler::uchar_type::obj;

void c_compiler::uchar_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned char";
  if ( !name.empty() )
    os << ' ' << name;
}

const c_compiler::type* c_compiler::uchar_type::promotion() const
{
  return int_type::create();
}

c_compiler::short_type c_compiler::short_type::obj;

int c_compiler::short_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::short_type::decl(std::ostream& os, std::string name) const
{
  os << "short int";
  if ( !name.empty() )
    os << ' ' << name;
}

const c_compiler::type* c_compiler::short_type::promotion() const
{
  return int_type::create();
}

c_compiler::ushort_type c_compiler::ushort_type::obj;

int c_compiler::ushort_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::ushort_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned short int";
  if ( !name.empty() )
    os << ' ' << name;
}

const c_compiler::type* c_compiler::ushort_type::promotion() const
{
  return int_type::create();
}

c_compiler::int_type c_compiler::int_type::obj;

int c_compiler::int_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::int_type::decl(std::ostream& os, std::string name) const
{
  os << "int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::uint_type c_compiler::uint_type::obj;

int c_compiler::uint_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::uint_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::long_type c_compiler::long_type::obj;

int c_compiler::long_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::long_type::decl(std::ostream& os, std::string name) const
{
  os << "long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::ulong_type c_compiler::ulong_type::obj;

int c_compiler::ulong_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::ulong_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::long_long_type c_compiler::long_long_type::obj;

int c_compiler::long_long_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::long_long_type::decl(std::ostream& os, std::string name) const
{
  os << "long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::ulong_long_type c_compiler::ulong_long_type::obj;

int c_compiler::ulong_long_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::ulong_long_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::float_type c_compiler::float_type::obj;

int c_compiler::float_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::float_type::decl(std::ostream& os, std::string name) const
{
  os << "float";
  if ( !name.empty() )
    os << ' ' << name;
}

const c_compiler::type* c_compiler::float_type::varg() const { return double_type::create(); }

c_compiler::double_type c_compiler::double_type::obj;

int c_compiler::double_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::double_type::decl(std::ostream& os, std::string name) const
{
  os << "double";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::long_double_type c_compiler::long_double_type::obj;

int c_compiler::long_double_type::size() const { return type_impl::sizeof_table[&obj]; }

void c_compiler::long_double_type::decl(std::ostream& os, std::string name) const
{
  os << "long double";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::backpatch_type c_compiler::backpatch_type::obj;

struct c_compiler::const_type::table_t : c_compiler::pmap<const type*, const const_type> {};

void c_compiler::const_type::decl(std::ostream& os, std::string name) const
{
  typedef const pointer_type PT;
  if ( m_T->m_id == type::POINTER ){
    name = "const " + name;
    m_T->decl(os,name);
  }
  else {
    os << "const ";
    m_T->decl(os,name);
  }
}

bool c_compiler::const_type::include_cvr(const type* that) const
{
  int x = 0;
  const type* a = this->unqualified(&x);
  int y = 0;
  const type* b = that->unqualified(&y);
  if ( ~x & y )
    return false;
  return a->include_cvr(b);
}

const c_compiler::type* c_compiler::const_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T,u));
}

const c_compiler::type* c_compiler::const_type::qualified(int cvr) const
{
  cvr |= 1;
  return m_T->qualified(cvr);
}

const c_compiler::const_type* c_compiler::const_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const const_type* ret = new const_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new const_type(T);
}

struct c_compiler::volatile_type::table_t : c_compiler::pmap<const type*, const volatile_type> {};

c_compiler::volatile_type::table_t c_compiler::volatile_type::table;

void c_compiler::volatile_type::decl(std::ostream& os, std::string name) const
{
  typedef const pointer_type PT;
  if ( m_T->m_id == type::POINTER ){
    name = "volatile " + name;
    m_T->decl(os,name);
  }
  else {
    os << "volatile ";
    m_T->decl(os,name);
  }
}

bool c_compiler::volatile_type::include_cvr(const type* that) const
{
  int x = 0;
  const type* a = this->unqualified(&x);
  int y = 0;
  const type* b = that->unqualified(&y);
  if ( ~x & y )
    return false;
  return a->include_cvr(b);
}

const c_compiler::type* c_compiler::volatile_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T,u));
}

const c_compiler::type* c_compiler::volatile_type::qualified(int cvr) const
{
  cvr |= 2;
  return m_T->qualified(cvr);
}

const c_compiler::volatile_type* c_compiler::volatile_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const volatile_type* ret = new volatile_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new volatile_type(T);
}

struct c_compiler::restrict_type::table_t : c_compiler::pmap<const type*, const restrict_type> {};

c_compiler::restrict_type::table_t c_compiler::restrict_type::table;

void c_compiler::restrict_type::decl(std::ostream& os, std::string name) const
{
  typedef const pointer_type PT;
  if ( m_T->m_id == type::POINTER ){
    name = "restrict " + name;
    m_T->decl(os,name);
  }
  else {
    os << "restrict ";
    m_T->decl(os,name);
  }
}

bool c_compiler::restrict_type::include_cvr(const type* that) const
{
  int x = 0;
  const type* a = this->unqualified(&x);
  int y = 0;
  const type* b = that->unqualified(&y);
  if ( ~x & y )
    return false;
  return a->include_cvr(b);
}

const c_compiler::type* c_compiler::restrict_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T,u));
}

const c_compiler::type* c_compiler::restrict_type::qualified(int cvr) const
{
  cvr |= 4;
  return m_T->qualified(cvr);
}

const c_compiler::restrict_type* c_compiler::restrict_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const restrict_type* ret = new restrict_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new restrict_type(T);
}

struct c_compiler::func_type::table_t
  : c_compiler::pmap<std::pair<std::pair<const type*, std::vector<const type*> >,bool>, const func_type> {};

c_compiler::func_type::table_t c_compiler::func_type::table;

void c_compiler::func_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  typedef const pointer_type PT;
  int cvr = 0;
  T = T->unqualified(&cvr);
  if ( T->m_id == type::POINTER ){
    PT* pt = static_cast<PT*>(T);
    const type* x = pt->referenced_type();
    const type* y = x->prev();
    if ( x != y ){
      ostringstream tmp;
      tmp << '(' << '*';
      if ( cvr & 1 )
        tmp << "const ";
      if ( cvr & 2 )
        tmp << "volatile ";
      if ( cvr & 2 )
        tmp << "restrict ";
      tmp << name;
      y->decl(os,tmp.str());
      post(os);
      os << ')';
      x->post(os);
      return;
    }
    T = T->qualified(cvr);
  }
  T->decl(os,name);
  post(os);
}

void c_compiler::func_type::post(std::ostream& os) const
{
  using namespace std;
  os << '(';
  typedef vector<const type*>::const_iterator IT;
  for ( IT p = m_param.begin() ; p != m_param.end() ; ++p ){
    if ( p != m_param.begin() )
      os << ',';
    (*p)->decl(os,"");
  }
  os << ')';
}

namespace c_compiler { namespace func_impl {
  bool compatible(const type* x, const type* y){ return x->compatible(y); }
  namespace old_new {
    bool compatible(const std::vector<const type*>&, const std::vector<const type*>&);
  } // end of namespace old_new
} } // end of namespace func_impl and c_compier

bool c_compiler::func_type::compatible(const type* T) const
{
  using namespace std;
  if ( this == T )
    return true;
  if ( T->m_id != type::FUNC )
    return false;
  typedef const func_type FUNC;
  FUNC* that = static_cast<FUNC*>(T);
  if ( !this->m_T->compatible(that->m_T) )
    return false;
  if ( this->m_old_style != that->m_old_style ){
    if ( this->m_old_style )
      return func_impl::old_new::compatible(this->m_param,that->m_param);
    else
      return func_impl::old_new::compatible(that->m_param,this->m_param);
  }
  if ( this->m_old_style || that->m_old_style )
    return true;
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return false;
  return mismatch(u.begin(),u.end(),v.begin(),func_impl::compatible) == make_pair(u.end(),v.end());
}

namespace c_compiler { namespace func_impl { namespace old_new {
  const ellipsis_type* olddecl_nodef(const std::vector<const type*>&);
  const type* varg_helper(const type* T){ return T->varg(); }
} } } // end of namespace old_new, func_impl and c_compier

bool c_compiler::func_impl::old_new::compatible(const std::vector<const type*>& o,
                                                const std::vector<const type*>& n)
{
  using namespace std;
  typedef const ellipsis_type ET;
  if ( ET* et = olddecl_nodef(o) ){
    if ( n.back()->m_id == type::ELLIPSIS )
      return false;
    vector<const type*> p;
#ifdef _MSC_VER
    transform(n.begin(),n.end(),back_inserter(p),varg_helper);
#else // _MSC_VER
    transform(n.begin(),n.end(),back_inserter(p),mem_fun(&type::varg));
#endif // _MSC_VER
    return mismatch(n.begin(),n.end(),p.begin(),func_impl::compatible) == make_pair(n.end(),p.end());
  }
  if ( o.size() != n.size() )
    return false;
  vector<const type*> p;
#ifdef _MSC_VER
  transform(o.begin(),o.end(),back_inserter(p),varg_helper);
#else // _MSC_VER
  transform(o.begin(),o.end(),back_inserter(p),mem_fun(&type::varg));
#endif // _MSC_VER
  return mismatch(n.begin(),n.end(),p.begin(),func_impl::compatible) == make_pair(n.end(),p.end());
}

const c_compiler::ellipsis_type*
c_compiler::func_impl::old_new::olddecl_nodef(const std::vector<const type*>& o)
{
  if ( o.size() != 1 )
    return 0;
  const type* T = o[0];
  if ( T->m_id != type::ELLIPSIS )
    return 0;
  typedef const ellipsis_type ET;
  ET* et = static_cast<ET*>(T);
  return et;
}

namespace c_compiler { namespace func_impl {
  const type* composite(const type* x, const type* y){ return x->composite(y); }
  namespace old_new {
    const func_type* composite(const func_type*, const func_type*);
  } // end of namespace old_new
} } // end of namespace func_impl and c_compier

const c_compiler::type* c_compiler::func_type::composite(const type* T) const
{
  using namespace std;
  if ( this == T )
    return this;
  if ( T->m_id != type::FUNC )
    return 0;
  typedef const func_type FUNC;
  FUNC* that = static_cast<FUNC*>(T);
  if ( !this->m_T->compatible(that->m_T) )
    return 0;
  if ( this->m_old_style != that->m_old_style ){
    if ( this->m_old_style )
      return func_impl::old_new::composite(this,that);
    else
      return func_impl::old_new::composite(that,this);
  }
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( this->m_old_style || that->m_old_style )
    return u.size() > v.size() ? this : that;
  if ( u.size() != v.size() )
    return 0;
  if ( mismatch(u.begin(),u.end(),v.begin(),func_impl::compatible) != make_pair(u.end(),v.end()) )
    return 0;
  vector<const type*> param;
  transform(u.begin(),u.end(),v.begin(),back_inserter(param),func_impl::composite);
  return create(this->m_T->composite(that->m_T),param,false);
}

const c_compiler::func_type*
c_compiler::func_impl::old_new::composite(const func_type* of, const func_type* nf)
{
  using namespace std;
  typedef const ellipsis_type ET;
  const vector<const type*>& o = of->param();
  const vector<const type*>& n = nf->param();
  if ( ET* et = olddecl_nodef(o) ){
    if ( n.back()->m_id == type::ELLIPSIS )
      return 0;
    vector<const type*> p;
#ifdef _MSC_VER
    transform(n.begin(),n.end(),back_inserter(p),varg_helper);
#else // _MSC_VER
    transform(n.begin(),n.end(),back_inserter(p),mem_fun(&type::varg));
#endif // _MSC_VER
    if ( mismatch(n.begin(),n.end(),p.begin(),func_impl::compatible) != make_pair(n.end(),p.end()) )
      return 0;
    vector<const type*> param;
    transform(n.begin(),n.end(),p.begin(),back_inserter(param),func_impl::composite);
    return func_type::create(of->return_type()->composite(nf->return_type()),param,false);
  }
  if ( o.size() != n.size() )
    return 0;
  vector<const type*> p;
#ifdef _MSC_VER
  transform(o.begin(),o.end(),back_inserter(p),varg_helper);
#else // _MSC_VER
  transform(o.begin(),o.end(),back_inserter(p),mem_fun(&type::varg));
#endif // _MSC_VER
  if ( mismatch(n.begin(),n.end(),p.begin(),func_impl::compatible) != make_pair(n.end(),p.end()) )
    return 0;
  vector<const type*> param;
  transform(n.begin(),n.end(),o.begin(),back_inserter(param),func_impl::composite);
  return func_type::create(of->return_type()->composite(nf->return_type()),param,false);
}

namespace c_compiler { namespace func_impl {
  bool include_cvr(const type* x, const type* y){ return x->include_cvr(y); }
  namespace old_new {
    bool include_cvr(const std::vector<const type*>&, const std::vector<const type*>&);
  } // end of namespace old_new
} } // end of namespace func_impl and c_compier

bool c_compiler::func_type::include_cvr(const type* T) const
{
  using namespace std;
  if ( this == T )
    return true;
  if ( T->m_id != type::FUNC )
    return false;
  typedef const func_type FUNC;
  FUNC* that = static_cast<FUNC*>(T);
  if ( !this->m_T->include_cvr(that->m_T) )
    return false;
  if ( this->m_old_style != that->m_old_style ){
    if ( this->m_old_style )
      return func_impl::old_new::include_cvr(this->m_param,that->m_param);
    else
      return func_impl::old_new::include_cvr(that->m_param,this->m_param);
  }
  if ( this->m_old_style || that->m_old_style )
    return true;
  const vector<const type*>& u = this->m_param;
  const vector<const type*>& v = that->m_param;
  if ( u.size() != v.size() )
    return false;
  return mismatch(u.begin(),u.end(),v.begin(),func_impl::include_cvr) == make_pair(u.end(),v.end());
}

bool
c_compiler::func_impl::old_new::include_cvr(const std::vector<const type*>& o,
                                            const std::vector<const type*>& n)
{
  using namespace std;
  typedef const ellipsis_type ET;
  if ( ET* et = olddecl_nodef(o) ){
    if ( n.back()->m_id == type::ELLIPSIS )
      return false;
    vector<const type*> p;
#ifdef _MSC_VER
    transform(n.begin(),n.end(),back_inserter(p),varg_helper);
#else // _MSC_VER
    transform(n.begin(),n.end(),back_inserter(p),mem_fun(&type::varg));
#endif // _MSC_VER
    return mismatch(n.begin(),n.end(),p.begin(),func_impl::include_cvr) == make_pair(n.end(),p.end());
  }
  if ( o.size() != n.size() )
    return false;
  vector<const type*> p;
#ifdef _MSC_VER
  transform(o.begin(),o.end(),back_inserter(p),varg_helper);
#else // _MSC_VER
  transform(o.begin(),o.end(),back_inserter(p),mem_fun(&type::varg));
#endif // _MSC_VER
  return mismatch(n.begin(),n.end(),p.begin(),func_impl::include_cvr) == make_pair(n.end(),p.end());
}

const c_compiler::type* c_compiler::func_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( T->m_id == type::FUNC ){
    using namespace error::decl::declarator;
    func::of_func(parse::position,u);
    T = int_type::create();
  }
  if ( T->m_id == type::ARRAY ){
    using namespace error::decl::declarator;
    func::of_array(parse::position,u);
    T = int_type::create();
  }
  if ( u ){
    usr::flag& flag = u->m_flag;
    flag = usr::flag(flag | usr::FUNCTION);
  }
  return create(T,m_param,m_old_style);
}

const c_compiler::type* c_compiler::func_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_param,m_old_style);
}

#ifdef _MSC_VER
namespace c_compiler { namespace func_impl {
  const type* complete_type(const type* T){ return T->complete_type(); }
} } // end of namespace func_impl and c_compiler
#endif // _MSC_VER

const c_compiler::type* c_compiler::func_type::complete_type() const
{
  using namespace std;
  const type* T = m_T->complete_type();
  vector<const type*> param;
#ifndef _MSC_VER
  transform(m_param.begin(),m_param.end(),back_inserter(param),mem_fun(&type::complete_type));
#else // _MSC_VER
  transform(m_param.begin(),m_param.end(),back_inserter(param),func_impl::complete_type);
#endif // _MSC_VER
  return create(T,param,m_old_style);
}

const c_compiler::pointer_type* c_compiler::func_type::ptr_gen() const
{
  return pointer_type::create(this);
}

bool c_compiler::func_type::temporary(bool b) const
{
  using namespace std;
  return m_T->temporary(b) || find_if(m_param.begin(), m_param.end(), bind2nd(mem_fun(&type::temporary), b)) != m_param.end();
}

const c_compiler::func_type* c_compiler::func_type::create(const type* T,
                                                           const std::vector<const type*>& param,
                                                           bool old_style)
{
  using namespace std;
  if (T->temporary(false) || find_if(param.begin(), param.end(), bind2nd(mem_fun(&type::temporary), false)) != param.end()) {
    const func_type* ret = new func_type(T,param,old_style);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  pair<pair<const type*, vector<const type*> >,bool> key(make_pair(T,param),old_style);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new func_type(T,param,old_style);
}

struct c_compiler::array_type::table_t : c_compiler::pmap<std::pair<const type*,int>, const array_type> {};

c_compiler::array_type::table_t c_compiler::array_type::table;

void c_compiler::array_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  typedef const pointer_type PT;
  int cvr = 0;
  T = T->unqualified(&cvr);
  if ( T->m_id == type::POINTER ){
    PT* pt = static_cast<PT*>(T);
    const type* x = pt->referenced_type();
    const type* y = x->prev();
    if ( x != y ){
      ostringstream tmp;
      tmp << '(' << '*';
      if ( cvr & 1 )
        tmp << "const ";
      if ( cvr & 2 )
        tmp << "volatile ";
      if ( cvr & 2 )
        tmp << "restrict ";
      tmp << name;
      y->decl(os,tmp.str());
      post(os);
      os << ')';
      x->post(os);
      return;
    }
    T = T->qualified(cvr);
  }
  T->decl(os,name);
  post(os);
}

bool c_compiler::array_type::compatible(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    if ( this->m_dim && that->m_dim && this->m_dim != that->m_dim )
      return false;
    return this->m_T->compatible(that->m_T);
  }
  else if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->element_type());
  }
  else
    return false;
}

const c_compiler::type* c_compiler::array_type::composite(const type* T) const
{
  if ( this == T )
    return this;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    if ( this->m_dim && that->m_dim && this->m_dim != that->m_dim  )
      return 0;
    if ( !this->m_T->compatible(that->m_T) )
      return 0;
    return this->m_dim ? this : that;
  }
  else if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->element_type()) ? that : 0;
  }
  else
    return 0;
}

bool c_compiler::array_type::include_cvr(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->include_cvr(that->m_T);
  }
  else if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->include_cvr(that->element_type());
  }
  else
    return false;
}

void c_compiler::array_type::post(std::ostream& os) const
{
  os << '[';
  if ( m_dim )
    os << m_dim;
  os << ']';
  m_T->post(os);
}

const c_compiler::type* c_compiler::array_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( T->m_id == type::FUNC ){
    using namespace error::decl::declarator;
    array::of_func(parse::position,u);
    T = backpatch_type::create();
  }
  T = T->complete_type();
  return create(T,m_dim);
}

const c_compiler::type* c_compiler::array_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_dim);
}

const c_compiler::type* c_compiler::array_type::complete_type() const
{
  return create(m_T->complete_type(),m_dim);
}

const c_compiler::pointer_type* c_compiler::array_type::ptr_gen() const
{
  return pointer_type::create(m_T);
}

std::pair<int, const c_compiler::type*> c_compiler::array_type::current(int nth) const
{
  using namespace std;
  return ( m_dim && m_dim <= nth ) ? make_pair(-1,static_cast<const type*>(0))
    : make_pair(m_T->size() * nth, m_T);
}

c_compiler::var* c_compiler::array_type::vsize() const
{
  using namespace std;
  var* size = m_T->vsize();
  if ( !size )
    return 0;
  usr* dim = integer::create(m_dim);
  return size->mul(dim);
}

const c_compiler::array_type* c_compiler::array_type::create(const type* T, int dim)
{
  using namespace std;
  if ( T->temporary(false) ){
    const array_type* ret = new array_type(T,dim);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  pair<const type*, int> key(T,dim);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new array_type(T,dim);
}

struct c_compiler::pointer_type::table_t : c_compiler::pmap<const type*, const pointer_type> {};

int c_compiler::pointer_type::size() const { return type_impl::m_pointer_sizeof; }

c_compiler::pointer_type::table_t c_compiler::pointer_type::table;

void c_compiler::pointer_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  ostringstream tmp;
  if ( T == m_T )
    tmp << '*' << name;
  else
    tmp << '(' << '*' << name << ')';
  m_T->decl(os,tmp.str());
}

const c_compiler::type* c_compiler::pointer_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( u ){
    usr::flag& flag = u->m_flag;
    flag = (usr::flag)(flag & ~usr::FUNCTION & ~usr::VL);
  }
  return create(T);
}

bool c_compiler::pointer_type::compatible(const type* T) const
{
  T = T->unqualified();
  if ( this == T )
    return true;
  typedef const pointer_type PT;
  if ( T->m_id == type::POINTER ){
    PT* that = static_cast<PT*>(T);
    return this->m_T->compatible(that->m_T);
  }
  else
    return false;
}

const c_compiler::type* c_compiler::pointer_type::composite(const type* T) const
{
  int cvr = 0;
  T = T->unqualified(&cvr);
  typedef const pointer_type PT;
  if ( T->m_id == type::POINTER ){
    PT* that = static_cast<PT*>(T);
    return this->m_T->compatible(that->m_T) ? create(this->m_T->composite(that->m_T))->qualified(cvr) : 0;
  }
  else
    return 0;
}

bool c_compiler::pointer_type::include_cvr(const type* T) const
{
  typedef const pointer_type PT;
  if ( T->m_id != type::POINTER )
    return false;
  PT* that = static_cast<PT*>(T);
  const type* a = this->m_T;
  const type* b = that->m_T;
  int c = 0;
  int d = 0;
  const type* e = a->unqualified(&c);
  const type* f = b->unqualified(&d);
  if ( ~c & d )
    return false;
  typedef const pointer_type PT;
  if ( e->m_id != type::POINTER )
    return a->include_cvr(b);
  PT* g = static_cast<PT*>(e);
  PT* h = static_cast<PT*>(f);
  if ( !g->include_cvr(h) )
    return false;
  const type* i = g->m_T;
  const type* j = h->m_T;
  int k = 0;
  int l = 0;
  i->unqualified(&k);
  j->unqualified(&l);
  if ( ~k & l )
    return false;
  if ( (k&4) && !(l&4) && !(c&1) )
    return false;
  if ( (k&2) && !(l&2) && !(c&1) )
    return false;
  if ( (k&1) && !(l&1) && !(c&1) )
    return false;
  return true;
}

const c_compiler::type* c_compiler::pointer_type::complete_type() const
{
  return create(m_T->complete_type());
}

const c_compiler::pointer_type* c_compiler::pointer_type::create(const type* T)
{
  if ( T->temporary(false) ){
    const pointer_type* ret = new pointer_type(T);
    if ( scope::current->m_id == scope::BLOCK )
      type_impl::temp1.push_back(ret);
#ifdef _DEBUG
    else
      type_impl::temp2.push_back(ret);
#endif // _DEBUG
    return ret;
  }
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new pointer_type(T);
}

void c_compiler::ellipsis_type::decl(std::ostream& os, std::string name) const
{
  os << "...";
  assert(name.empty());
}

c_compiler::ellipsis_type c_compiler::ellipsis_type::m_obj;

const c_compiler::ellipsis_type* c_compiler::ellipsis_type::create()
{
  return &m_obj;
}

void c_compiler::incomplete_tagged_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

bool c_compiler::incomplete_tagged_type::compatible(const type* T) const
{
  const type* that = T->unqualified();
  if ( this == that )
    return true;
  if ( that->m_id == type::RECORD ){
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(that);
    return m_tag == rec->get_tag();
  }
  if ( that->m_id == type::ENUM ){
    typedef const enum_type ENUM;
    ENUM* et = static_cast<ENUM*>(that);
    return m_tag == et->get_tag();
  }
  return false;
}

const c_compiler::type* c_compiler::incomplete_tagged_type::composite(const type* T) const
{
  const type* that = T->unqualified();
  if ( this == that )
    return this;
  if ( that->m_id == type::RECORD ){
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(that);
    return m_tag == rec->get_tag() ? rec : 0;
  }
  if ( that->m_id == type::ENUM ){
    typedef const enum_type ENUM;
    ENUM* et = static_cast<ENUM*>(that);
    return m_tag == et->get_tag() ? et : 0;
  }
  return 0;
}

const c_compiler::type* c_compiler::incomplete_tagged_type::complete_type() const
{
  return m_tag->m_types.second ? m_tag->m_types.second : this;
}

bool c_compiler::incomplete_tagged_type::temporary(bool vm) const
{
  if ( vm )
    return false;
  else
    return m_tag->m_scope != &scope::root;
}

const c_compiler::incomplete_tagged_type* c_compiler::incomplete_tagged_type::create(tag* tag)
{
  return new incomplete_tagged_type(tag);
}

namespace c_compiler { namespace record_impl {
  struct layouter {
    std::insert_iterator<std::map<std::string, std::pair<int, usr*> > > X;
    std::insert_iterator<std::map<usr*, int> > Y;
    usr* m_last;
    struct current {
      usr* m_member;
      const type* m_integer;
      int m_position;
      current(usr* member = 0, const type* T = 0)
        : m_member(member), m_integer(T), m_position(0) {}
    } m_current;
    int operator()(int, usr*);
    layouter(std::insert_iterator<std::map<std::string, std::pair<int, usr*> > > XX,
             std::insert_iterator<std::map<usr*, int> > YY,
             usr* last)
             : X(XX), Y(YY), m_last(last) {}
  };
  struct grounder {
    std::insert_iterator<std::map<usr*, int> > Y;
    std::pair<std::string, std::pair<int, usr*> > operator()(usr*);
    grounder(std::insert_iterator<std::map<usr*, int> > YY) : Y(YY) {}
  };
  bool comp_size(usr*, usr*);
  bool comp_align(usr*, usr*);
  bool member_modifiable(usr*);
} } // end of namespace record_impl and c_compiler

c_compiler::record_type::record_type(tag* tag, const std::vector<usr*>& member)
  : type(RECORD), m_tag(tag), m_member(member)
{
  using namespace std;
  using namespace record_impl;
  if ( m_member.empty() )
    m_member.push_back(new usr("",char_type::create(),usr::NONE,parse::position));
  if ( tag->m_kind == tag::STRUCT ){
    usr* last = *m_member.rbegin();
    m_size = accumulate(m_member.begin(),m_member.end(),0,
                        layouter(inserter(m_layout,m_layout.begin()),inserter(m_position,m_position.begin()),last));
    const type* T = m_member.back()->m_type;
    if ( T->m_id == type::BIT_FIELD ){
      typedef const bit_field_type BF;
      BF* bf = static_cast<BF*>(T);
      T = bf->integer_type();
      m_size += T->size();
      usr::flag& flag = m_member.back()->m_flag;
      flag = usr::flag(flag | usr::MSB_FIELD);
    }
  }
  else {
    assert(tag->m_kind == tag::UNION);
    transform(m_member.begin(),m_member.end(),inserter(m_layout,m_layout.begin()),
      grounder(inserter(m_position,m_position.begin())));
    {
      vector<usr*>::const_iterator p = max_element(m_member.begin(),m_member.end(),comp_size);
      const type* T = (*p)->m_type;
      if ( T->m_id == type::BIT_FIELD ){
        typedef const bit_field_type BF;
        BF* bf = static_cast<BF*>(T);
        T = bf->integer_type();
      }
      m_size = T->size();
    }
  }
  m_modifiable =
    find_if(m_member.begin(),m_member.end(),not1(ptr_fun(member_modifiable))) == m_member.end();

  int al = align();
  if ( int n = m_size % al ) {
    m_size += al - n;
  }
}

int c_compiler::record_impl::layouter::operator()(int offset, usr* member)
{
  using namespace std;
  if ( member->m_flag & usr::BIT_FIELD ){
    const type* T = member->m_type;
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    T = bf->integer_type();
    bool update = false;
    if ( const type* C = m_current.m_integer ){
      if ( C != T ){
        usr::flag& flag = m_current.m_member->m_flag;
        flag = usr::flag(flag | usr::MSB_FIELD);
        offset += C->size();
        m_current = current(member,T);
      }
      else if ( m_current.m_position + bf->bit() > T->size() * 8 ){
        offset += C->size();
        usr::flag& flag = m_current.m_member->m_flag;
        flag = usr::flag(flag | usr::MSB_FIELD);
        m_current = current(member,T);
      }
      else {
        m_current.m_member = member;
        update = true;
      }
    }
    else {
      m_current = current(member,T);
      update = true;
    }
    int align = T->align();
    if ( int n = offset % align )
      offset += align - n;
    string name = member->m_name;
    *X++ = make_pair(name,make_pair(offset,member));
    *Y++ = make_pair(member,m_current.m_position);
    if ( update )
      m_current.m_position += bf->bit();
    return offset;
  }
  else {
    if ( const type* C = m_current.m_integer ){
      usr::flag& flag = m_current.m_member->m_flag;
      flag = usr::flag(flag | usr::MSB_FIELD);
      offset += C->size();
    }
    m_current = current();
    string name = member->m_name;
    const type* T = member->m_type;
    if ( T->temporary(true) ){
      using namespace error::decl::struct_or_union;
      not_ordinary(member);
      T = member->m_type = int_type::create();
    }
    if ( !T->size() ){
      int n = code.size();
      typedef const array_type ARRAY;
      ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
      if ( member == m_last && array && !array->vsize() ){
        T = array->element_type();
        int align = T->align();
        assert(align);
        if ( int n = offset % align )
          offset += align - n;
        *X++ = make_pair(name,make_pair(offset,member));
        return offset;
      }
      for_each(code.begin()+n,code.end(),deleter<tac>());
      code.resize(n);
      using namespace error::decl::struct_or_union;
      incomplete_or_function(member);
      T = member->m_type = int_type::create();
    }
    int align = T->align();
    assert(align);
    if ( int n = offset % align )
      offset += align - n;
    *X++ = make_pair(name,make_pair(offset,member));
    return offset + T->size();
  }
}

std::pair<std::string, std::pair<int, c_compiler::usr*> >
c_compiler::record_impl::grounder::operator()(usr* member)
{
  using namespace std;
  const type* T = member->m_type;
  if ( !T->size() ){
    using namespace error::decl::struct_or_union;
    incomplete_or_function(member);
    T = member->m_type = int_type::create();
  }
  if ( T->m_id == type::BIT_FIELD ){
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    *Y++ = make_pair(member,0);
  }
  string name = member->m_name;
  return make_pair(name,make_pair(0,member));
}

bool c_compiler::record_impl::comp_size(usr* x, usr* y)
{
  typedef const bit_field_type BF;
  const type* xx = x->m_type;
  if ( xx->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(xx);
    xx = bf->integer_type();
  }
  const type* yy = y->m_type;
  if ( yy->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(yy);
    yy = bf->integer_type();
  }
  return xx->size() < yy->size();
}

bool c_compiler::record_impl::comp_align(usr* x, usr* y)
{
  typedef const bit_field_type BF;
  const type* xx = x->m_type;
  if ( xx->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(xx);
    xx = bf->integer_type();
  }
  const type* yy = y->m_type;
  if ( yy->m_id == type::BIT_FIELD ){
    BF* bf = static_cast<BF*>(yy);
    yy = bf->integer_type();
  }
  return xx->align() < yy->align();
}

bool c_compiler::record_impl::member_modifiable(usr* member)
{
  return member->m_type->modifiable();
}

void c_compiler::record_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

bool c_compiler::record_type::compatible(const type* T) const
{
  const type* that = T->unqualified();
  if ( this == that )
    return true;
  else if ( that->m_id == type::INCOMPLETE_TAGGED ){
    typedef const incomplete_tagged_type ITT;
    ITT* itt = static_cast<ITT*>(that);
    return m_tag == itt->get_tag();
  }
  else
    return false;
}

const c_compiler::type* c_compiler::record_type::composite(const type* T) const
{
  const type* that = T->unqualified();
  if ( this == that )
    return this;
  else if ( that->m_id == type::INCOMPLETE_TAGGED ){
    typedef const incomplete_tagged_type ITT;
    ITT* itt = static_cast<ITT*>(that);
    return m_tag == itt->get_tag() ? this : 0;
  }
  else
    return 0;
}

std::pair<int, c_compiler::usr*> c_compiler::record_type::offset(std::string name) const
{
  using namespace std;
  map<string, pair<int, usr*> >::const_iterator p = m_layout.find(name);
  if ( p != m_layout.end() )
    return p->second;
  else
    return make_pair(-1,static_cast<usr*>(0));
}

int c_compiler::record_type::position(usr* member) const
{
  using namespace std;
  map<usr*, int>::const_iterator p = m_position.find(member);
  assert(p != m_position.end());
  return p->second;
}

std::pair<int, const c_compiler::type*> c_compiler::record_type::current(int nth) const
{
  using namespace std;
  tag* tg = get_tag();
  if ( tg->m_kind == tag::UNION && nth >= 1 )
    return make_pair(-1,static_cast<const type*>(0));
  if ( m_member.size() <= nth )
    return make_pair(-1,static_cast<const type*>(0));
  usr* u = m_member[nth];
  const type* T = u->m_type;
  string name = u->m_name;
  map<string, pair<int, usr*> >::const_iterator p = m_layout.find(name);
  assert(p != m_layout.end());
  int offset = p->second.first;
  return make_pair(offset,T);
}

bool c_compiler::record_type::temporary(bool vm) const
{
  if ( vm )
    return false;
  else
    return m_tag->m_scope != &scope::root;
}

const c_compiler::record_type* c_compiler::record_type::create(tag* tag, const std::vector<usr*>& member)
{
  return new record_type(tag,member);
}

c_compiler::record_type::~record_type()
{
  using namespace std;
  for_each(m_member.begin(),m_member.end(),deleter<usr>());
}

void c_compiler::enum_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

bool c_compiler::enum_type::compatible(const type* that) const
{
  that = that->unqualified();
  that = that->complete_type();
  return this == that;
}

const c_compiler::type* c_compiler::enum_type::composite(const type* that) const
{
  int cvr = 0;
  that = that->unqualified(&cvr);
  that = that->complete_type();
  return this == that ? qualified(cvr) : 0;
}

bool c_compiler::enum_type::temporary(bool vm) const
{
  if ( vm )
    return false;
  else
    return m_tag->m_scope != &scope::root;
}

const c_compiler::enum_type* c_compiler::enum_type::create(tag* tag, const type* integer)
{
  return new enum_type(tag,integer);
}

struct c_compiler::bit_field_type::table_t : c_compiler::pmap<std::pair<int, const type*>, const bit_field_type> {};

c_compiler::bit_field_type::table_t c_compiler::bit_field_type::table;

const c_compiler::type* c_compiler::bit_field_type::patch(const type* T, usr* u) const
{
  using namespace error::decl::struct_or_union::bit_field;
  if ( !T->integer() ){
    not_integer_type(u);
    T = int_type::create();
  }
  usr::flag& flag = u->m_flag;
  flag = usr::flag(flag | usr::BIT_FIELD);
  int n = T->size();
  n <<= 3;
  int bit = m_bit;
  if ( bit > n ){
    exceed(u,T);
    bit = 1;
  }
  if ( !bit && !u->m_name.empty() ){
    zero(u);
    bit = 1;
  }
  return create(bit,m_integer->patch(T,u));
}

const c_compiler::bit_field_type* c_compiler::bit_field_type::create(int bit, const type* integer)
{
  using namespace std;
  table_t::const_iterator p = table.find(make_pair(bit,integer));
  if ( p != table.end() )
    return p->second;
  else
    return table[make_pair(bit,integer)] = new bit_field_type(bit,integer);
}

c_compiler::varray_type::varray_type(const type* T, var* dim)
 : type(VARRAY), m_T(T), m_decided(false)
{
  m_dim.first = dim;
  m_dim.second = new var(dim->m_type);
  m_dim.second->m_scope = 0;
}

c_compiler::varray_type::varray_type(const type* T, var* dim, const std::vector<tac*>& c)
 : type(VARRAY), m_T(T), m_decided(false), m_code(c)
{
  m_dim.first = dim;
  m_dim.second = new var(dim->m_type);
  m_dim.second->m_scope = 0;
}

c_compiler::varray_type::~varray_type()
{
  using namespace std;
  if ( !m_decided )
    delete m_dim.second;
  for_each(m_code.begin(),m_code.end(),deleter<tac>());
}

void c_compiler::varray_type::decl(std::ostream& os, std::string name) const
{
  using namespace std;
  const type* T = m_T->prev();
  typedef const pointer_type PT;
  int cvr = 0;
  T = T->unqualified(&cvr);
  if ( T->m_id == type::POINTER ){
    PT* pt = static_cast<PT*>(T);
    const type* x = pt->referenced_type();
    const type* y = x->prev();
    if ( x != y ){
      ostringstream tmp;
      tmp << '(' << '*';
      if ( cvr & 1 )
        tmp << "const ";
      if ( cvr & 2 )
        tmp << "volatile ";
      if ( cvr & 2 )
        tmp << "restrict ";
      tmp << name;
      y->decl(os,tmp.str());
      post(os);
      os << ')';
      x->post(os);
      return;
    }
    T = T->qualified(cvr);
  }
  T->decl(os,name);
  post(os);
}

bool c_compiler::varray_type::compatible(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->m_T);
  }
  else if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->compatible(that->element_type());
  }
  else
    return false;
}

const c_compiler::type* c_compiler::varray_type::composite(const type* T) const
{
  if ( this == T )
    return this;
  if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->m_T) ? this : 0;
  }
  else if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->compatible(that->element_type()) ? this : 0;
  }
  else
    return 0;
}

bool c_compiler::varray_type::include_cvr(const type* T) const
{
  if ( this == T )
    return true;
  if ( T->m_id == type::VARRAY ){
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->include_cvr(that->m_T);
  }
  else if ( T->m_id == type::ARRAY ){
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->include_cvr(that->element_type());
  }
  else
    return false;
}

void c_compiler::varray_type::post(std::ostream& os) const
{
  os << '[';
  os << ']';
  m_T->post(os);
}

const c_compiler::type* c_compiler::varray_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( T->m_id == type::FUNC ){
    using namespace error::decl::declarator;
    array::of_func(parse::position,u);
    T = backpatch_type::create();
  }
  T = T->complete_type();
  if ( u ){
    usr::flag& flag = u->m_flag;
    flag = usr::flag(flag | usr::VL);
  }
  const type* ret = m_code.empty() ? create(T,m_dim.first) : create(T,m_dim.first,m_code);
  m_code.clear();
  return ret;
}

const c_compiler::type* c_compiler::varray_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_dim.first);
}

const c_compiler::type* c_compiler::varray_type::complete_type() const
{
  m_T = m_T->complete_type();
  return this;
}

const c_compiler::pointer_type* c_compiler::varray_type::ptr_gen() const
{
  return pointer_type::create(m_T);
}

namespace c_compiler { namespace varray_impl {
  void move1(tac*);
  void move2(var*);
} } // end of namespace varray_impl and c_compiler

void c_compiler::varray_impl::move1(tac* ptr)
{
  if ( ptr->x ) move2(ptr->x);
  if ( ptr->y ) move2(ptr->y);
  if ( ptr->z ) move2(ptr->z);
}

void c_compiler::varray_impl::move2(var* v)
{
  using namespace std;
  vector<var*>::reverse_iterator p = find(garbage.rbegin(),garbage.rend(),v);
  if ( p != garbage.rend() ){
    garbage.erase(p.base()-1);
    block* b = static_cast<block*>(scope::current);
    v->m_scope = b;
    b->m_vars.push_back(v);
  }
}

void c_compiler::varray_type::decide() const
{
  using namespace std;
  if ( m_decided )
    return;
  m_T->decide();
  for_each(m_code.begin(),m_code.end(),varray_impl::move1);
  copy(m_code.begin(),m_code.end(),back_inserter(code));
  m_code.clear();
  block* b = static_cast<block*>(scope::current);
  m_dim.second->m_scope = b;
  b->m_vars.push_back(m_dim.second);
  code.push_back(new assign3ac(m_dim.second,m_dim.first));
  m_decided = true;
}

c_compiler::var* c_compiler::varray_type::vsize() const
{
  using namespace std;
  const type* T = m_T;
  T = T->complete_type();
  if ( var* vs = T->vsize() )
    return m_dim.second->mul(vs);
  else {
    int n = T->size();
    usr* size = integer::create(n);
    return m_dim.second->mul(size);
  }
}

const c_compiler::varray_type* c_compiler::varray_type::create(const type* T, var* dim)
{
  using namespace std;
  const varray_type* ret = new varray_type(T,dim);
  if ( scope::current->m_id == scope::BLOCK )
    type_impl::temp1.push_back(ret);
#ifdef _DEBUG
  else
    type_impl::temp2.push_back(ret);
#endif // _DEBUG
  return ret;
}

const c_compiler::varray_type* c_compiler::varray_type::create(const type* T, var* dim, const std::vector<tac*>& c)
{
  using namespace std;
  const varray_type* ret = new varray_type(T,dim,c);
  if ( scope::current->m_id == scope::BLOCK )
    type_impl::temp1.push_back(ret);
#ifdef _DEBUG
  else
    type_impl::temp2.push_back(ret);
#endif // _DEBUG
  return ret;
}
