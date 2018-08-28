#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

namespace c_compiler { namespace expr { namespace cast_impl {
  const type* valid(const type*, var*);
} } } // end of namespace cast_impl, expr and c_compiler

c_compiler::var* c_compiler::expr::cast(const type* T, var* expr)
{
  using namespace std;
  expr = expr->rvalue();
  if ( T->compatible(void_type::create()) ){
    var* ret = new var(void_type::create());
    garbage.push_back(ret);
    return ret;
  }
  if ( !T->scalar() ){
    using namespace error::expr::cast;
    not_scalar(parse::position);
    T = int_type::create();
  }
  T = cast_impl::valid(T,expr);
  if ( !T ){
    using namespace error::expr::cast;
    invalid(parse::position);
    T = int_type::create();
  }
  return expr->cast(T);
}

const c_compiler::type* c_compiler::expr::cast_impl::valid(const type* T, var* y)
{
  if ( const type* r = assign_impl::valid(T,y,0) )
    return r;
  const type* Tx = T->unqualified();
  const type* Ty = y->m_type->unqualified();
  if ( Tx->m_id == type::POINTER )
    return Ty->real() ? 0 : T;
  if ( Ty->m_id == type::POINTER )
    return Tx->real() ? 0 : T;
  return 0;
}

c_compiler::var* c_compiler::var::cast(const type* T)
{
  if ( T == m_type && !lvalue() )
    return this;
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  if ( T->compatible(m_type) )
    code.push_back(new assign3ac(ret,this));
  else
    code.push_back(new cast3ac(ret,this,T));
  return ret;
}

c_compiler::var* c_compiler::addrof::cast(const type* T)
{
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expr::constant_flag )
    return var::cast(T);
  if ( T == m_type )
    return this;
  else {
    var* ret = new addrof(T,m_ref,m_offset);
    garbage.push_back(ret);
    return ret;
  }
}

namespace c_compiler { namespace constant_impl {
  var* cast_ld(long double);
} } // end of namespace constant_impl and c_compiler

c_compiler::var* c_compiler::constant_impl::cast_ld(long double ld)
{
  if ( generator::long_double ){
    int sz = long_double_type::create()->size();
    unsigned char* p = new unsigned char[sz];
    (*generator::long_double->from_double)(p,ld);
    return floating::create(p);
  }
  else
    return floating::create(ld);
}

namespace c_compiler { namespace constant_impl {
  template<class T> var* cast(const type* type, constant<T>* y)
  {
    using namespace std;
    if (type->compatible(y->m_type))
      return y;
    else if (type->compatible(char_type::create()))
      return integer::create((char)(y->m_value));
    else if (type->compatible(schar_type::create()))
      return integer::create((signed char)(y->m_value));
    else if (type->compatible(uchar_type::create()))
      return integer::create((unsigned char)(y->m_value));
    else if (type->compatible(short_type::create()))
      return integer::create((short int)(y->m_value));
    else if (type->compatible(ushort_type::create()))
      return integer::create((unsigned short int)(y->m_value));
    else if (type->compatible(int_type::create()))
      return integer::create((int)(y->m_value));
    else if (type->compatible(uint_type::create()))
      return integer::create((unsigned int)(y->m_value));
    else if (type->compatible(long_type::create())) {
	typedef long int X;
	if (type->size() <= sizeof(X))
	  return integer::create((X)(y->m_value));
	typedef long long int XX;
	assert(type->size() == sizeof(XX));
	return integer::create((XX)(y->m_value));
    }
    else if (type->compatible(ulong_type::create())) {
	typedef unsigned long int X;
	if (type->size() <= sizeof(X))
	  return integer::create((X)(y->m_value));
	typedef unsigned long long int XX;
	assert(type->size() == sizeof(XX));
	return integer::create((XX)(y->m_value));
    }
    else if (type->compatible(long_long_type::create()))
      return integer::create((__int64)(y->m_value));
    else if (type->compatible(ulong_long_type::create()))
      return integer::create((unsigned __int64)(y->m_value));
    else if (type->compatible(float_type::create()))
      return floating::create((float)y->m_value);
    else if (type->compatible(double_type::create()))
      return floating::create((double)y->m_value);
    else if (type->compatible(long_double_type::create()))
      return cast_ld(y->m_value);
    else if (type->m_id == type::POINTER ){
      if (sizeof(void*) >= type->size())
	return pointer::create(type,(void*)y->m_value);
      else
	return pointer::create(type,(__int64)y->m_value);
    }
    else if (type->m_id == type::ENUM){
      typedef const enum_type ET;
      ET* et = static_cast<ET*>(type);
      return cast(et->get_integer(),y);
    }
    else
      return y->var::cast(type);
  }
  template<class T> var* fcast(const type* type, constant<T>* y)
  {
    using namespace std;
    if ( type->compatible(y->m_type) )
      return y;
    else if ( type->compatible(char_type::create()) )
      return integer::create((char)(y->m_value));
    else if ( type->compatible(schar_type::create()) )
      return integer::create((signed char)(y->m_value));
    else if ( type->compatible(uchar_type::create()) )
      return integer::create((unsigned char)(y->m_value));
    else if ( type->compatible(short_type::create()) )
      return integer::create((short int)(y->m_value));
    else if ( type->compatible(ushort_type::create()) )
      return integer::create((unsigned short int)(y->m_value));
    else if ( type->compatible(int_type::create()) )
      return integer::create((int)(y->m_value));
    else if ( type->compatible(uint_type::create()) )
      return integer::create((unsigned int)(y->m_value));
    else if ( type->compatible(long_type::create()) )
      return integer::create((long int)(y->m_value));
    else if ( type->compatible(ulong_type::create()) )
      return integer::create((unsigned long int)(y->m_value));
    else if ( type->compatible(long_long_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(ulong_long_type::create()) )
      return integer::create((unsigned __int64)(y->m_value));
    else if ( type->compatible(float_type::create()) )
      return floating::create((float)y->m_value);
    else if ( type->compatible(double_type::create()) )
      return floating::create((double)y->m_value);
    else if ( type->compatible(long_double_type::create()) )
      return cast_ld(y->m_value);
    else if ( type->m_id == type::ENUM ){
      typedef const enum_type ET;
      ET* et = static_cast<ET*>(type);
      return fcast(et->get_integer(),y);
    }
    else
      return y->var::cast(type);
  }
  template<class T> var* pcast(const type* type, constant<T>* y)
  {
    using namespace std;
    if ( type->compatible(y->m_type) )
      return y;
    else if ( type->compatible(char_type::create()) )
      return integer::create((char)(__int64)y->m_value);
    else if ( type->compatible(schar_type::create()) )
      return integer::create((signed char)(__int64)y->m_value);
    else if ( type->compatible(uchar_type::create()) )
      return integer::create((unsigned char)(__int64)y->m_value);
    else if ( type->compatible(short_type::create()) )
      return integer::create((short int)(__int64)y->m_value);
    else if ( type->compatible(ushort_type::create()) )
      return integer::create((unsigned short int)(__int64)y->m_value);
    else if ( type->compatible(int_type::create()) )
      return integer::create((int)(__int64)y->m_value);
    else if ( type->compatible(uint_type::create()) )
      return integer::create((unsigned int)(__int64)y->m_value);
    else if ( type->compatible(long_type::create()) )
      return integer::create((long int)y->m_value);
    else if ( type->compatible(ulong_type::create()) )
      return integer::create((unsigned long int)y->m_value);
    else if ( type->compatible(long_long_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(ulong_long_type::create()) )
      return integer::create((unsigned __int64)y->m_value);
    else if ( type->m_id == type::POINTER ) {
      if (sizeof(void*) >= type->size())
	return pointer::create(type,(void*)y->m_value);
      else
	return pointer::create(type,(__int64)y->m_value);
    }
    else if ( type->m_id == type::ENUM ){
      typedef const enum_type ET;
      ET* et = static_cast<ET*>(type);
      return pcast(et->get_integer(),y);
    }
    else
      return y->var::cast(type);
  }
#ifdef _MSC_VER
  template<> var* cast(const type* type, constant<unsigned __int64>* y)
  {
    using namespace std;
    if ( type->compatible(y->m_type) )
      return y;
    else if ( type->compatible(char_type::create()) )
      return integer::create((char)(y->m_value));
    else if ( type->compatible(schar_type::create()) )
      return integer::create((signed char)(y->m_value));
    else if ( type->compatible(uchar_type::create()) )
      return integer::create((unsigned char)(y->m_value));
    else if ( type->compatible(short_type::create()) )
      return integer::create((short int)(y->m_value));
    else if ( type->compatible(ushort_type::create()) )
      return integer::create((unsigned short int)(y->m_value));
    else if ( type->compatible(int_type::create()) )
      return integer::create((int)(y->m_value));
    else if ( type->compatible(uint_type::create()) )
      return integer::create((unsigned int)(y->m_value));
    else if ( type->compatible(long_type::create()) )
      return integer::create((long int)(y->m_value));
    else if ( type->compatible(ulong_type::create()) )
      return integer::create((unsigned long int)(y->m_value));
    else if ( type->compatible(long_long_type::create()) )
      return integer::create((__int64)(y->m_value));
    else if ( type->compatible(ulong_long_type::create()) )
      return integer::create((unsigned __int64)(y->m_value));
    else if ( type->compatible(float_type::create()) )
      return floating::create((float)(__int64)y->m_value);
    else if ( type->compatible(double_type::create()) )
      return floating::create((double)(__int64)y->m_value);
    else if ( type->compatible(long_double_type::create()) )
      return cast_ld((__int64)y->m_value);
    else if ( type->m_id == type::POINTER ) {
      if (sizeof(void*) >= type->size())
	return pointer::create(type,(void*)y->m_value);
      else
	return pointer::create(type,(__int64)y->m_value);
    }
    else if ( type->m_id == type::ENUM ){
      typedef const enum_type ET;
      ET* et = static_cast<ET*>(type);
      return cast(et->get_integer(),y);
    }
    else
      return y->var::cast(type);
  }
#endif // _MSC_VER
} } // end of namespace constant_impl and c_compmiler

namespace c_compiler {
  template<> var* constant<char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<signed char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<unsigned char>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<unsigned short int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<unsigned int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<unsigned long int>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<__int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
  template<> var* constant<unsigned __int64>::cast(const type* type)
  { return constant_impl::cast(type,this); }
} // end of namespace c_compiler

c_compiler::var* c_compiler::constant<float>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
c_compiler::var* c_compiler::constant<double>::cast(const type* type)
{ return constant_impl::fcast(type,this); }
c_compiler::var* c_compiler::constant<long double>::cast(const type* type)
{
  if ( generator::long_double ){
    if ( type->compatible(long_double_type::create()) )
      return this;
    else {
      double d = (*generator::long_double->to_double)(b);
      usr* tmp = floating::create(d);
      return tmp->cast(type);
    }
  }
  else
    return constant_impl::fcast(type,this);
}
c_compiler::var* c_compiler::constant<void*>::cast(const type* type)
{ return constant_impl::pcast(type,this); }

c_compiler::var* c_compiler::expr::_va_start(var* x, var* y)
{
  using namespace error::expr::va;
  x = x->rvalue();
  if ( !x->lvalue() )
    invalid("va_start", parse::position, x);
  const type* Tx = x->m_type;
  if ( !Tx->modifiable() )
    invalid("va_start",parse::position,x);
  Tx = Tx->unqualified();
  if ( Tx->m_id != type::POINTER )
    invalid("va_start",parse::position,x);
  y = y->rvalue();
  if ( !y->lvalue() )
    not_lvalue(parse::position);
  const type* Ty = y->m_type;
  int n = Ty->size();
  if ( !n )
    no_size(parse::position);
  code.push_back(new va_start3ac(x, y));
  return x;
}

c_compiler::var* c_compiler::expr::_va_arg(var* v, const type* T)
{
  v = v->rvalue();
  {
    using namespace error::expr::va;
    const type* T = v->m_type;
    if ( !T->modifiable() )
      invalid("va_arg",parse::position,v);
    T = T->unqualified();
    if ( T->m_id != type::POINTER )
      invalid("va_arg",parse::position,v);
  }
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new va_arg3ac(ret,v,T));
  return ret;
}

c_compiler::var* c_compiler::expr::_va_end(var* y)
{
  y = y->rvalue();
  code.push_back(new va_end3ac(y));
  return y;
}

namespace c_compiler {
  template<> var* refimm<void*>::common()
  {
    if (sizeof(void*) == sizeof(int)) {
      int i = (int)(__int64)m_addr;
      return integer::create(i);
    }
    return integer::create((__int64)m_addr);
  }
  template<> var* refimm<__int64>::common()
  {
    return integer::create(m_addr);
  }
} // end of namespace c_compiler

