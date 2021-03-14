#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"

namespace c_compiler { namespace type_impl {
    using namespace std;
    struct sizeof_table_t : map<const type*, int> {
      sizeof_table_t();
    } sizeof_table;
    int pointer_sizeof = sizeof(void*);
} } // end of namespace type_impl and c_compiler

c_compiler::type_impl::sizeof_table_t::sizeof_table_t()
{
  (*this)[short_type::create()] =
  (*this)[ushort_type::create()] = sizeof(short);
  (*this)[int_type::create()] =
  (*this)[uint_type::create()] = sizeof(int);
  (*this)[long_type::create()] =
  (*this)[ulong_type::create()] = sizeof(long);
  (*this)[long_long_type::create()] =
  (*this)[ulong_long_type::create()] = sizeof(long long);
  (*this)[float_type::create()] = sizeof(float);
  (*this)[double_type::create()] = sizeof(double);
  (*this)[long_double_type::create()] = sizeof(long double);
}

void c_compiler::type_impl::update(int (*size)(int id))
{
  if (!size)
    return;
  sizeof_table[short_type::create()] =
  sizeof_table[ushort_type::create()] = size((int)type::SHORT);
  sizeof_table[int_type::create()] =
  sizeof_table[uint_type::create()] = size((int)type::INT);
  sizeof_table[long_type::create()] =
  sizeof_table[ulong_type::create()] = size((int)type::LONG);
  sizeof_table[long_long_type::create()] =
  sizeof_table[ulong_long_type::create()] = size((int)type::LONGLONG);
  sizeof_table[float_type::create()] = size((int)type::FLOAT);
  sizeof_table[double_type::create()] = size((int)type::DOUBLE);
  sizeof_table[long_double_type::create()] = size((int)type::LONG_DOUBLE);
  pointer_sizeof = size((int)type::POINTER);
}

bool c_compiler::type::compatible(const type* that) const
{
  return this == that;
}

const c_compiler::type* c_compiler::type::composite(const type* that) const
{
  return this == that ? this : 0;
}

std::pair<int, const c_compiler::type*> c_compiler::type::current(int nth) const
{
  using namespace std;
  const type* zero = 0;
  return nth ? make_pair(-1, zero) : make_pair(0, this);
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

void c_compiler::type::destroy_tmp()
{
  const_type::destroy_tmp();
  volatile_type::destroy_tmp();
  restrict_type::destroy_tmp();
  func_type::destroy_tmp();
  array_type::destroy_tmp();
  pointer_type::destroy_tmp();
  incomplete_tagged_type::destroy_tmp();
  record_type::destroy_tmp();
  enum_type::destroy_tmp();
  varray_type::destroy_tmp();
}

void c_compiler::type::collect_tmp(std::vector<const type*>& vt)
{
  const_type::collect_tmp(vt);
  volatile_type::collect_tmp(vt);
  restrict_type::collect_tmp(vt);
  func_type::collect_tmp(vt);
  array_type::collect_tmp(vt);
  pointer_type::collect_tmp(vt);
  incomplete_tagged_type::collect_tmp(vt);
  record_type::collect_tmp(vt);
  enum_type::collect_tmp(vt);
  varray_type::collect_tmp(vt);
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

int c_compiler::short_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

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

int c_compiler::ushort_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

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

int c_compiler::int_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::int_type::decl(std::ostream& os, std::string name) const
{
  os << "int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::uint_type c_compiler::uint_type::obj;

int c_compiler::uint_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::uint_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::long_type c_compiler::long_type::obj;

int c_compiler::long_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::long_type::decl(std::ostream& os, std::string name) const
{
  os << "long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::ulong_type c_compiler::ulong_type::obj;

int c_compiler::ulong_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::ulong_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::long_long_type c_compiler::long_long_type::obj;

int c_compiler::long_long_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::long_long_type::decl(std::ostream& os, std::string name) const
{
  os << "long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::ulong_long_type c_compiler::ulong_long_type::obj;

int c_compiler::ulong_long_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::ulong_long_type::decl(std::ostream& os, std::string name) const
{
  os << "unsigned long long int";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::float_type c_compiler::float_type::obj;

int c_compiler::float_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::float_type::decl(std::ostream& os, std::string name) const
{
  os << "float";
  if ( !name.empty() )
    os << ' ' << name;
}

const c_compiler::type* c_compiler::float_type::varg() const
{
  return double_type::create();
}

c_compiler::double_type c_compiler::double_type::obj;

int c_compiler::double_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void c_compiler::double_type::decl(std::ostream& os, std::string name) const
{
  os << "double";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::long_double_type c_compiler::long_double_type::obj;

int c_compiler::long_double_type::size() const
{
  return type_impl::sizeof_table[&obj];
}

void
c_compiler::long_double_type::decl(std::ostream& os, std::string name) const
{
  os << "long double";
  if ( !name.empty() )
    os << ' ' << name;
}

c_compiler::backpatch_type c_compiler::backpatch_type::obj;

namespace c_compiler {
  const_type::table_t const_type::tmp_tbl;
  const_type::table_t const_type::pmt_tbl;
} // end of namespace c_compiler

void c_compiler::const_type::decl(std::ostream& os, std::string name) const
{
  if (m_T->m_id == type::POINTER) {
    name = "const " + name;
    m_T->decl(os, name);
  }
  else {
    os << "const ";
    m_T->decl(os,name);
  }
}

bool c_compiler::const_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != CONST)
    return false;
  typedef const const_type CT;
  CT* that = static_cast<CT*>(T);
  return this->m_T->compatible(that->m_T);
}

const c_compiler::type*
c_compiler::const_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != CONST)
    return 0;
  typedef const const_type CT;
  CT* that = static_cast<CT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
}

const c_compiler::type*
c_compiler::const_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T, u));
}

const c_compiler::type*
c_compiler::const_type::qualified(int cvr) const
{
  cvr |= 1;
  return m_T->qualified(cvr);
}

const c_compiler::type*
c_compiler::const_type::create(const type* T)
{
  if (T->m_id == CONST)
    return T;

  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
      return p->second;
  return table[T] = new const_type(T);
}

void c_compiler::const_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void c_compiler::const_type::collect_tmp(std::vector<const type*>& vs)
{
  for (auto p : tmp_tbl)
    vs.push_back(p.second);
  tmp_tbl.clear();
}


namespace c_compiler {
  volatile_type::table_t volatile_type::tmp_tbl;
  volatile_type::table_t volatile_type::pmt_tbl;
} // end of namespace c_compiler

void c_compiler::volatile_type::decl(std::ostream& os, std::string name) const
{
  if ( m_T->m_id == type::POINTER ){
    name = "volatile " + name;
    m_T->decl(os,name);
  }
  else {
    os << "volatile ";
    m_T->decl(os,name);
  }
}

bool c_compiler::volatile_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != VOLATILE)
    return false;
  typedef const volatile_type VT;
  VT* that = static_cast<VT*>(T);
  return this->m_T->compatible(that->m_T);
}

const c_compiler::type*
c_compiler::volatile_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != VOLATILE)
    return 0;
  typedef const volatile_type VT;
  VT* that = static_cast<VT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
}

const c_compiler::type*
c_compiler::volatile_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T, u));
}

const c_compiler::type* c_compiler::volatile_type::qualified(int cvr) const
{
  cvr |= 2;
  return m_T->qualified(cvr);
}

const c_compiler::type*
c_compiler::volatile_type::create(const type* T)
{
  if (T->m_id == VOLATILE)
    return T;

  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if (p != table.end())
      return p->second;
  
  if (T->m_id == CONST) {
    typedef const const_type CT;
    CT* ct = static_cast<CT*>(T);
    volatile_type* vt = new volatile_type(ct->m_T);
    table[ct->m_T] = vt;
    return const_type::create(vt);
  }

  volatile_type* ret = new volatile_type(T);
  return table[T] = ret;
}

void c_compiler::volatile_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void c_compiler::volatile_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}

namespace c_compiler {
  restrict_type::table_t restrict_type::tmp_tbl;
  restrict_type::table_t restrict_type::pmt_tbl;
} // end of namespace c_compiler

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

bool c_compiler::restrict_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != RESTRICT)
    return false;
  typedef const restrict_type RT;
  RT* that = static_cast<RT*>(T);
  return this->m_T->compatible(that->m_T);
}

const c_compiler::type*
c_compiler::restrict_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != RESTRICT)
    return 0;
  typedef const restrict_type RT;
  RT* that = static_cast<RT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
}

const c_compiler::type*
c_compiler::restrict_type::patch(const type* T, usr* u) const
{
  return create(m_T->patch(T, u));
}

const c_compiler::type* c_compiler::restrict_type::qualified(int cvr) const
{
  cvr |= 4;
  return m_T->qualified(cvr);
}

const c_compiler::type*
c_compiler::restrict_type::create(const type* T)
{
  if (T->m_id == RESTRICT)
    return T;

  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if (p != table.end())
    return p->second;

  typedef const const_type CT;  
  typedef const volatile_type VT;
  
  if (T->m_id == CONST) {
    CT* ct = static_cast<CT*>(T);
    const type* T2 = ct->m_T;
    if (T2->m_id == VOLATILE) {
      VT* vt = static_cast<VT*>(T2);
      restrict_type* rt = new restrict_type(vt->m_T);
      table[vt->m_T] = rt;
      return const_type::create(volatile_type::create(rt));
    }
    restrict_type* rt = new restrict_type(T2);
    table[T2] = rt;
    return const_type::create(rt);
  }

  if (T->m_id == VOLATILE) {
    VT* vt = static_cast<VT*>(T);
    restrict_type* rt = new restrict_type(vt->m_T);
    table[vt->m_T] = rt;
    return volatile_type::create(rt);
  }

  restrict_type* ret = new restrict_type(T);  
  return table[T] = ret;
}

void c_compiler::restrict_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void c_compiler::restrict_type::collect_tmp(std::vector<const type*>& vs)
{
  for (auto p : tmp_tbl)
    vs.push_back(p.second);
  tmp_tbl.clear();
}

namespace c_compiler {
  using namespace std;
  struct func_type::table_t
  : map<pair<pair<const type*, vector<const type*> >,bool>, const func_type*>
  {};
  func_type::table_t func_type::tmp_tbl;
  func_type::table_t func_type::pmt_tbl;
} // end of namespace c_compiler

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
      if ( cvr & 4 )
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
  else {
    if (cvr & 1)
      os << "const ";
    if (cvr & 2)
      os << "volatile ";
    if (cvr & 4)
      os << "restrict ";
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

namespace c_compiler {
  namespace func_impl {
    namespace old_new {
      using namespace std;
      bool compatible(const vector<const type*>&, const vector<const type*>&);
    } // end of namespace old_new
  } // end of namespace func_impl
} // end of namespace c_compier

bool c_compiler::func_type::compatible(const type* T) const
{
  using namespace std;
  if ( this == T )
    return true;
  if ( T->m_id != type::FUNC )
    return false;
  typedef const func_type FT;
  FT* that = static_cast<FT*>(T);
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
  if (u.size() != v.size())
    return false;
  return mismatch(u.begin(),u.end(),v.begin(),c_compiler::compatible) == make_pair(u.end(),v.end());
}

namespace c_compiler { namespace func_impl { namespace old_new {
  const ellipsis_type* olddecl_nodef(const std::vector<const type*>&);
} } } // end of namespace old_new, func_impl and c_compier

bool
c_compiler::func_impl::old_new::compatible(const std::vector<const type*>& o,
                                           const std::vector<const type*>& n)
{
  using namespace std;
  typedef const ellipsis_type ET;
  if (ET* et = olddecl_nodef(o)) {
    if (n.back()->m_id == type::ELLIPSIS)
      return false;
    vector<const type*> p;
    transform(n.begin(),n.end(),back_inserter(p),mem_fun(&type::varg));
    return mismatch(n.begin(),n.end(),p.begin(),c_compiler::compatible) == make_pair(n.end(),p.end());
  }
  if (o.size() != n.size())
    return false;
  vector<const type*> p;
  transform(o.begin(),o.end(),back_inserter(p),mem_fun(&type::varg));
  return mismatch(n.begin(),n.end(),p.begin(),c_compiler::compatible) == make_pair(n.end(),p.end());
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

namespace c_compiler {
  namespace func_impl {
    namespace old_new {
      const func_type* composite(const func_type*, const func_type*);
    } // end of namespace old_new
  } // end of namespace func_impl
} // end of namespace c_compier

const c_compiler::type* c_compiler::func_type::composite(const type* T) const
{
  using namespace std;
  if (this == T)
    return this;
  if (T->m_id != type::FUNC)
    return 0;
  typedef const func_type FUNC;
  FUNC* that = static_cast<FUNC*>(T);
  if (!this->m_T->compatible(that->m_T))
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
  if (u.size() != v.size())
    return 0;
  if (mismatch(u.begin(),u.end(),v.begin(),c_compiler::compatible) != make_pair(u.end(),v.end()))
    return 0;
  vector<const type*> param;
  transform(u.begin(),u.end(),v.begin(),back_inserter(param),c_compiler::composite);
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
    transform(n.begin(),n.end(),back_inserter(p),mem_fun(&type::varg));
    if (mismatch(n.begin(),n.end(),p.begin(),c_compiler::compatible) != make_pair(n.end(),p.end()))
      return 0;
    vector<const type*> param;
    transform(n.begin(),n.end(),p.begin(),back_inserter(param),c_compiler::composite);
    return func_type::create(of->return_type()->composite(nf->return_type()),param,false);
  }
  if (o.size() != n.size())
    return 0;
  vector<const type*> p;
  transform(o.begin(),o.end(),back_inserter(p),mem_fun(&type::varg));
  if (mismatch(n.begin(),n.end(),p.begin(),c_compiler::compatible) != make_pair(n.end(),p.end()))
    return 0;
  vector<const type*> param;
  transform(n.begin(),n.end(),o.begin(),back_inserter(param),c_compiler::composite);
  return func_type::create(of->return_type()->composite(nf->return_type()),param,false);
}

const c_compiler::type*
c_compiler::func_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if ( T->m_id == type::FUNC ){
    using namespace error::decl::declarator;
    func::of_func(parse::position,u);
    if (T->backpatch())
      T = backpatch_type::create();
    else
      T = int_type::create();
  }
  if ( T->m_id == type::ARRAY ){
    using namespace error::decl::declarator;
    func::of_array(parse::position,u);
    if (T->backpatch())
      T = backpatch_type::create();
    else
      T = int_type::create();
  }
  if (u) {
    usr::flag_t& flag = u->m_flag;
    flag = usr::flag_t(flag | usr::FUNCTION);
  }
  return create(T,m_param,m_old_style);
}

const c_compiler::type* c_compiler::func_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_param,m_old_style);
}

const c_compiler::type* c_compiler::func_type::complete_type() const
{
  using namespace std;
  const type* T = m_T->complete_type();
  vector<const type*> param;
  transform(m_param.begin(),m_param.end(),back_inserter(param),mem_fun(&type::complete_type));
  return create(T,param,m_old_style);
}

const c_compiler::pointer_type* c_compiler::func_type::ptr_gen() const
{
  return pointer_type::create(this);
}

bool c_compiler::func_type::tmp() const
{
  using namespace std;
  return m_T->tmp() || find_if(m_param.begin(), m_param.end(), mem_fun(&type::tmp)) != m_param.end();
}

bool c_compiler::func_type::variably_modified() const
{
  using namespace std;
  return m_T->variably_modified() || find_if(m_param.begin(), m_param.end(), mem_fun(&type::variably_modified)) != m_param.end();
}

const c_compiler::type* c_compiler::func_type::vla2a() const
{
  using namespace std;
  const type* T = m_T->vla2a();
  vector<const type*> param;
  transform(m_param.begin(), m_param.end(), back_inserter(param), mem_fun(&type::vla2a));
  return create(T, param, m_old_style);
}

const c_compiler::func_type* c_compiler::func_type::create(const type* T,
                                                           const std::vector<const type*>& param,
                                                           bool old_style)
{
  using namespace std;
  table_t& table = (T->tmp() || find_if(param.begin(), param.end(), mem_fun(&type::tmp)) != param.end()) ? tmp_tbl : pmt_tbl;
  pair<pair<const type*, vector<const type*> >,bool> key(make_pair(T,param),old_style);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new func_type(T,param,old_style);
}

void c_compiler::func_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void c_compiler::func_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}

namespace c_compiler {
  using namespace std;
  struct array_type::table_t
    : map<pair<const type*,int>, const array_type*> {};
  array_type::table_t array_type::tmp_tbl;
  array_type::table_t array_type::pmt_tbl;
} // end of namespace c_compiler

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
      if ( cvr & 4 )
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
  else {
    if (cvr & 1)
      os << "const ";
    if (cvr & 2)
      os << "volatile ";
    if (cvr & 4)
      os << "restrict ";
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

const c_compiler::type*
c_compiler::array_type::composite(const type* T) const
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

void c_compiler::array_type::post(std::ostream& os) const
{
  os << '[';
  if ( m_dim )
    os << m_dim;
  os << ']';
  m_T->post(os);
}

const c_compiler::type*
c_compiler::array_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if (T->m_id == type::FUNC) {
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
  return expr::binary('*', size, dim);
}

const c_compiler::array_type*
c_compiler::array_type::create(const type* T, int dim)
{
  using namespace std;
  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  pair<const type*, int> key(T,dim);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new array_type(T,dim);
}

void c_compiler::array_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void c_compiler::array_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
}

namespace c_compiler {
  pointer_type::table_t pointer_type::tmp_tbl;
  pointer_type::table_t pointer_type::pmt_tbl;
} // end of namespace c_compiler

int c_compiler::pointer_type::size() const
{
  return type_impl::pointer_sizeof;
}

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

const c_compiler::type*
c_compiler::pointer_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if (u)
    u->m_flag = usr::flag_t(u->m_flag & ~usr::FUNCTION & ~usr::VL);
  return create(T);
}

bool c_compiler::pointer_type::compatible(const type* T) const
{
  if (this == T)
    return true;
  if (T->m_id != type::POINTER)
    return false;
  typedef const pointer_type PT;
  PT* that = static_cast<PT*>(T);
  return this->m_T->compatible(that->m_T);
}

const c_compiler::type*
c_compiler::pointer_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id != type::POINTER)
    return 0;
  typedef const pointer_type PT;
  PT* that = static_cast<PT*>(T);
  T = this->m_T->composite(that->m_T);
  return T ? create(T) : 0;
}

const c_compiler::type* c_compiler::pointer_type::complete_type() const
{
  return create(m_T->complete_type());
}

const c_compiler::pointer_type*
c_compiler::pointer_type::create(const type* T)
{
  table_t& table = T->tmp() ? tmp_tbl : pmt_tbl;
  table_t::const_iterator p = table.find(T);
  if ( p != table.end() )
    return p->second;
  else
    return table[T] = new pointer_type(T);
}

void c_compiler::pointer_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p.second;
  tmp_tbl.clear();
}

void c_compiler::pointer_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p.second);
  tmp_tbl.clear();
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
  if (this == T)
    return true;
  if (T->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* that = static_cast<REC*>(T);
    return m_tag == that->get_tag();
  }
  if (T->m_id == type::ENUM) {
    typedef const enum_type ENUM;
    ENUM* that = static_cast<ENUM*>(T);
    return m_tag == that->get_tag();
  }
  return false;
}

const c_compiler::type*
c_compiler::incomplete_tagged_type::composite(const type* T) const
{
  if (this == T)
    return this;
  if (T->m_id == type::RECORD) {
    typedef const record_type REC;
    REC* that = static_cast<REC*>(T);
    return m_tag == that->get_tag() ? that : 0;
  }
  if (T->m_id == type::ENUM) {
    typedef const enum_type ENUM;
    ENUM* that = static_cast<ENUM*>(T);
    return m_tag == that->get_tag() ? that : 0;
  }
  return 0;
}

const c_compiler::type*
c_compiler::incomplete_tagged_type::complete_type() const
{
  return m_tag->m_types.second ? m_tag->m_types.second : this;
}

namespace c_compiler {
  bool inblock(const scope* ptr)
  {
    if (ptr->m_id == scope::BLOCK)
      return true;
    return ptr->m_parent ? inblock(ptr->m_parent) : false;
  }
  inline bool temporary(const tag* ptr)
  {
    return inblock(ptr->m_scope);
  }
} // end of namespace c_compiler

bool c_compiler::incomplete_tagged_type::tmp() const
{
  return tmp_tbl.find(this) != tmp_tbl.end();
}

namespace c_compiler {
  incomplete_tagged_type::table_t incomplete_tagged_type::tmp_tbl;
} // end of namespace c_compiler

const c_compiler::incomplete_tagged_type*
c_compiler::incomplete_tagged_type::create(tag* ptr)
{
  typedef incomplete_tagged_type ITT;
  ITT* ret = new ITT(ptr);
  if (temporary(ptr))
    tmp_tbl.insert(ret);
  return ret;
}

void c_compiler::incomplete_tagged_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p;
  tmp_tbl.clear();
}

void c_compiler::incomplete_tagged_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p);
  tmp_tbl.clear();
}

namespace c_compiler {
  namespace record_impl {
    using namespace std;    
    struct layouter {
      insert_iterator<map<string, pair<int, usr*> > > X;
      insert_iterator<map<usr*, int> > Y;
      usr* m_last;
      struct current {
        usr* m_member;
        const type* m_integer;
        int m_position;
        current(usr* member = 0, const type* T = 0)
          : m_member(member), m_integer(T), m_position(0) {}
      } m_current;
      int operator()(int, usr*);
      layouter(insert_iterator<map<string, pair<int, usr*> > > XX,
               insert_iterator<map<usr*, int> > YY,
               usr* last)
        : X(XX), Y(YY), m_last(last) {}
    };
    struct grounder {
      insert_iterator<map<usr*, int> > Y;
      pair<string, pair<int, usr*> > operator()(usr*);
      grounder(insert_iterator<map<usr*, int> > YY) : Y(YY) {}
    };
    bool comp_size(usr*, usr*);
    bool comp_align(usr*, usr*);
    bool member_modifiable(usr*, bool);
  } // end of namespace record_impl
} // end of namespace c_compiler

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
      usr::flag_t& flag = m_member.back()->m_flag;
      flag = usr::flag_t(flag | usr::MSB_FIELD);
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
  typedef vector<usr*>::const_iterator IT;
  IT p = find_if(m_member.begin(),m_member.end(),
		 not1(bind2nd(ptr_fun(member_modifiable), false)));
  m_modifiable = (p == m_member.end());
  IT q = find_if(m_member.begin(),m_member.end(),
		 bind2nd(ptr_fun(member_modifiable), true));
  m_partially_modifiable = (q != m_member.end());

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
    assert(T->m_id == type::BIT_FIELD);
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    T = bf->integer_type();
    bool update = false;
    if ( const type* C = m_current.m_integer ){
      if ( C != T ){
        usr::flag_t& flag = m_current.m_member->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
        offset += C->size();
        m_current = current(member,T);
      }
      else if ( m_current.m_position + bf->bit() > T->size() * 8 ){
        offset += C->size();
        usr::flag_t& flag = m_current.m_member->m_flag;
        flag = usr::flag_t(flag | usr::MSB_FIELD);
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
      usr::flag_t& flag = m_current.m_member->m_flag;
      flag = usr::flag_t(flag | usr::MSB_FIELD);
      offset += C->size();
    }
    m_current = current();
    string name = member->m_name;
    const type* T = member->m_type;
    if (T->variably_modified()) {
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
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
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

bool c_compiler::record_impl::member_modifiable(usr* member, bool partially)
{
  return member->m_type->modifiable(partially);
}

void c_compiler::record_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

bool c_compiler::record_type::compatible(const type* T) const
{
  if (this == T)
    return true;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return false;

  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag();
}

const c_compiler::type*
c_compiler::record_type::composite(const type* T) const
{
  if (this == T)
    return this;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return 0;
  
  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag() ? this : 0;
}

std::pair<int, c_compiler::usr*>
c_compiler::record_type::offset(std::string name) const
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

bool c_compiler::record_type::tmp() const
{
  return tmp_tbl.find(this) != tmp_tbl.end();
}

c_compiler::record_type::~record_type()
{
  for (auto p : m_member)
    delete p;
}

namespace c_compiler {
  record_type::table_t record_type::tmp_tbl;
} // end of namespace c_compiler

const c_compiler::record_type*
c_compiler::record_type::create(tag* ptr, const std::vector<usr*>& member)
{
  record_type* ret = new record_type(ptr,member);
  if (temporary(ptr))
    tmp_tbl.insert(ret);
  return ret;
}

void c_compiler::record_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p;
  tmp_tbl.clear();
}

void c_compiler::record_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p);
  tmp_tbl.clear();
}

void c_compiler::enum_type::decl(std::ostream& os, std::string name) const
{
  os << tag::keyword(m_tag->m_kind) << ' ' << m_tag->m_name;
  if ( !name.empty() )
    os << ' ' << name;
}

bool c_compiler::enum_type::compatible(const type* T) const
{
  if (this == T)
    return true;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return false;

  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag();
}

const c_compiler::type* c_compiler::enum_type::composite(const type* T) const
{
  if (this == T)
    return this;

  if (T->m_id != type::INCOMPLETE_TAGGED)
    return 0;

  typedef const incomplete_tagged_type ITT;
  ITT* that = static_cast<ITT*>(T);
  return m_tag == that->get_tag() ? this : 0;
}

bool c_compiler::enum_type::tmp() const
{
  return tmp_tbl.find(this) != tmp_tbl.end();
}

namespace c_compiler {
  enum_type::table_t enum_type::tmp_tbl;
} // end of namespace c_compiler

const c_compiler::enum_type*
c_compiler::enum_type::create(tag* ptr, const type* integer)
{
  enum_type* ret = new enum_type(ptr, integer);
  if (temporary(ptr))
    tmp_tbl.insert(ret);
  return ret;
}

void c_compiler::enum_type::destroy_tmp()
{
  for (auto p : tmp_tbl)
    delete p;
  tmp_tbl.clear();
}

void c_compiler::enum_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : tmp_tbl)
    vt.push_back(p);
  tmp_tbl.clear();
}

namespace c_compiler {
  using namespace std;
  struct bit_field_type::table_t :
    map<pair<int, const type*>, const bit_field_type*> {};
  bit_field_type::table_t bit_field_type::table;
} // end of namespace c_compiler

const c_compiler::type*
c_compiler::bit_field_type::patch(const type* T, usr* u) const
{
  using namespace error::decl::struct_or_union::bit_field;
  if ( !T->integer() ){
    not_integer_type(u);
    T = int_type::create();
  }
  u->m_flag = usr::flag_t(u->m_flag | usr::BIT_FIELD);
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

namespace c_compiler {
  using namespace std;
  struct varray_type::table_t
    : map<pair<const type*, var*>, const varray_type*> {};
  varray_type::table_t varray_type::table;
} // end of namespace c_compiler

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

  if (T->m_id == type::VARRAY) {
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->m_T);
  }

  if (T->m_id == type::ARRAY) {
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->compatible(that->element_type());
  }

  return false;
}

const c_compiler::type*
c_compiler::varray_type::composite(const type* T) const
{
  if ( this == T )
    return this;
  
  if (T->m_id == type::VARRAY) {
    typedef const varray_type VARRAY;
    VARRAY* that = static_cast<VARRAY*>(T);
    return this->m_T->compatible(that->m_T) ? this : 0;
  }
  
  if (T->m_id == type::ARRAY) {
    typedef const array_type ARRAY;
    ARRAY* that = static_cast<ARRAY*>(T);
    return this->m_T->compatible(that->element_type()) ? this : 0;
  }

  return 0;
}

void c_compiler::varray_type::post(std::ostream& os) const
{
  os << '[';
  os << ']';
  m_T->post(os);
}

const c_compiler::type*
c_compiler::varray_type::patch(const type* T, usr* u) const
{
  T = m_T->patch(T,u);
  if (T->m_id == type::FUNC) {
    using namespace error::decl::declarator;
    array::of_func(parse::position,u);
    T = backpatch_type::create();
  }
  T = T->complete_type();
  if (u) {
    usr::flag_t& flag = u->m_flag;
    flag = usr::flag_t(flag | usr::VL);
  }
  return create(T,m_dim);
}

const c_compiler::type* c_compiler::varray_type::qualified(int cvr) const
{
  return create(m_T->qualified(cvr),m_dim);
}

const c_compiler::type* c_compiler::varray_type::complete_type() const
{
  return create(m_T->complete_type(),m_dim);
}

const c_compiler::pointer_type* c_compiler::varray_type::ptr_gen() const
{
  return pointer_type::create(m_T);
}

c_compiler::var* c_compiler::varray_type::vsize() const
{
  using namespace std;
  const type* T = m_T->complete_type();
  if ( var* vs = T->vsize() )
    return expr::binary('*', m_dim, vs);

  int n = T->size();
  usr* size = integer::create(n);
  return expr::binary('*', m_dim, size);
}

const c_compiler::varray_type*
c_compiler::varray_type::create(const type* T, var* dim)
{
  using namespace std;
  pair<const type*, var*> key(T,dim);
  table_t::const_iterator p = table.find(key);
  if ( p != table.end() )
    return p->second;
  else
    return table[key] = new varray_type(T,dim);
}

void c_compiler::varray_type::destroy_tmp()
{
  for (auto p : table)
    delete p.second;
  table.clear();
}

void c_compiler::varray_type::collect_tmp(std::vector<const type*>& vt)
{
  for (auto p : table)
    vt.push_back(p.second);
  table.clear();
}
