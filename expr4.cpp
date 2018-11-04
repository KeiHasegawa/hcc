#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

namespace c_compiler { namespace var_impl {
  var* bitwise(int, var*, var*);
  var* bit_and(var*, var*);
} } // end of namespace var_impl and c_compiler

c_compiler::var* c_compiler::var_impl::bitwise(int op, var* a, var* b)
{
  decl::check(b);
  var* y = a->rvalue();
  var* z = b->rvalue();
  const type* Ty = y->m_type;
  const type* Tz = z->m_type;
  const type* T;
  if ( Ty->integer() && Tz->integer() )
    T = conversion::arithmetic(&y,&z);
  else {
    using namespace error::expr::binary;
    invalid(parse::position,op,y->m_type,z->m_type);
    T = int_type::create();
  }
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  switch ( op ){
  case '&': code.push_back(new and3ac(x,y,z)); break;
  case '^': code.push_back(new xor3ac(x,y,z)); break;
  case '|': code.push_back(new or3ac(x,y,z)); break;
  }
  return x;
}

c_compiler::var* c_compiler::var_impl::bit_and(var* a, var* b)
{
  return bitwise('&',a,b);
}

c_compiler::var* c_compiler::var::bit_and(var* z){ return var_impl::bit_and(this,z); }
c_compiler::var* c_compiler::var::bit_andr(constant<char>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<signed char>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<unsigned char>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<short int>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<unsigned short int>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<int>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<unsigned int>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<long int>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<unsigned long int>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<__int64>* y){ return var_impl::bit_and(y,this); }
c_compiler::var* c_compiler::var::bit_andr(constant<unsigned __int64>* y){ return var_impl::bit_and(y,this); }

namespace c_compiler { namespace constant_impl {
  template<class A, class B> var* bit_and(constant<A>* y, constant<B>* z)
  {
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return var_impl::bit_and(y,z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return var_impl::bit_and(y,z);
    usr* ret = integer::create(y->m_value & z->m_value);
    if (const type* T = SUB_CONST_LONG_impl::propagation(y, z))
      ret->m_type = T, ret->m_flag = usr::SUB_CONST_LONG;
    return ret;
  }
} } // end of namespace constant_impl and c_compiler

namespace c_compiler {
  template<> var* constant<char>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<char>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<signed char>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned char>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<short int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned short int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<long int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned long int>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<__int64>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<signed char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<unsigned char>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<unsigned short int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<unsigned int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<unsigned long int>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<__int64>* y)
  { return constant_impl::bit_and(y,this); }
  template<> var* constant<unsigned __int64>::bit_andr(constant<unsigned __int64>* y)
  { return constant_impl::bit_and(y,this); }
} // end of namespace c_compiler

namespace c_compiler { namespace var_impl {
  var* bit_xor(var*, var*);
} } // end of namespace var_impl and c_compiler

c_compiler::var* c_compiler::var_impl::bit_xor(var* a, var* b)
{
  return bitwise('^',a,b);
}

c_compiler::var* c_compiler::var::bit_xor(var* z){ return var_impl::bit_xor(this,z); }
c_compiler::var* c_compiler::var::bit_xorr(constant<char>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<signed char>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<unsigned char>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<short int>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<unsigned short int>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<int>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<unsigned int>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<long int>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<unsigned long int>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<__int64>* y){ return var_impl::bit_xor(y,this); }
c_compiler::var* c_compiler::var::bit_xorr(constant<unsigned __int64>* y){ return var_impl::bit_xor(y,this); }

namespace c_compiler { namespace constant_impl {
  template<class A, class B> var* bit_xor(constant<A>* y, constant<B>* z)
  {
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return var_impl::bit_xor(y,z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return var_impl::bit_xor(y,z);
    usr* ret = integer::create(y->m_value ^ z->m_value);
    if (const type* T = SUB_CONST_LONG_impl::propagation(y, z))
      ret->m_type = T, ret->m_flag = usr::SUB_CONST_LONG;
    return ret;
  }
} } // end of namespace constant_impl and c_compiler

namespace c_compiler {
  template<> var* constant<char>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<char>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<signed char>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned char>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<short int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned short int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<long int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned long int>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<__int64>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<signed char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<unsigned char>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<unsigned short int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<unsigned int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<unsigned long int>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<__int64>* y)
  { return constant_impl::bit_xor(y,this); }
  template<> var* constant<unsigned __int64>::bit_xorr(constant<unsigned __int64>* y)
  { return constant_impl::bit_xor(y,this); }
} // end of namespace c_compiler

namespace c_compiler { namespace var_impl {
  var* bit_or(var*, var*);
} } // end of namespace var_impl and c_compiler

c_compiler::var* c_compiler::var_impl::bit_or(var* a, var* b)
{
  return bitwise('|',a,b);
}

c_compiler::var* c_compiler::var::bit_or(var* z){ return var_impl::bit_or(this,z); }
c_compiler::var* c_compiler::var::bit_orr(constant<char>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<signed char>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<unsigned char>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<short int>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<unsigned short int>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<int>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<unsigned int>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<long int>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<unsigned long int>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<__int64>* y){ return var_impl::bit_or(y,this); }
c_compiler::var* c_compiler::var::bit_orr(constant<unsigned __int64>* y){ return var_impl::bit_or(y,this); }

namespace c_compiler { namespace constant_impl {
  template<class A, class B> var* bit_or(constant<A>* y, constant<B>* z)
  {
    usr::flag_t fy = y->m_flag;
    if (fy & usr::CONST_PTR)
      return var_impl::bit_or(y,z);
    usr::flag_t fz = z->m_flag;
    if (fz & usr::CONST_PTR)
      return var_impl::bit_or(y,z);
    usr* ret = integer::create(y->m_value | z->m_value);
    if (const type* T = SUB_CONST_LONG_impl::propagation(y, z))
      ret->m_type = T, ret->m_flag = usr::SUB_CONST_LONG;
    return ret;
  }
} } // end of namespace constant_impl and c_compiler

namespace c_compiler {
  template<> var* constant<char>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<char>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<signed char>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned char>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<short int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned short int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<long int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned long int>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<__int64>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<signed char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<unsigned char>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<unsigned short int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<unsigned int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<unsigned long int>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<__int64>* y)
  { return constant_impl::bit_or(y,this); }
  template<> var* constant<unsigned __int64>::bit_orr(constant<unsigned __int64>* y)
  { return constant_impl::bit_or(y,this); }
} // end of namespace c_comiler

namespace c_compiler { namespace SUB_CONST_LONG_impl {
  const type* propagation(const usr* y, const usr* z)
  {
    usr::flag_t fy = y->m_flag;
    usr::flag_t fz = z->m_flag;
    if (!(fy & usr::SUB_CONST_LONG) && !(fz & usr::SUB_CONST_LONG))
      return 0;

    const type* Ty = y->m_type;
    const type* Tz = z->m_type;
    const type* Tyq = Ty->unqualified();
    const type* Tzq = Tz->unqualified();
    type::id_t iy = Tyq->m_id;
    type::id_t iz = Tzq->m_id;

    if ((fy & usr::SUB_CONST_LONG) && (fz & usr::SUB_CONST_LONG)) {
      if (iy == type::ULONG)
        return Ty;
      if (iz == type::ULONG)
        return Tz;
      assert(iy == type::LONG && iz == type::LONG);
      return Ty;
    }
    
    if (fz & usr::SUB_CONST_LONG)
      return propagation(z, y);
    
    assert(fy & usr::SUB_CONST_LONG);
    assert(!(fz & usr::SUB_CONST_LONG));
    assert(Tz->integer());
    if (iz == type::ULONGLONG || iz == type::LONGLONG)
      return 0;
    assert(iz != type::ULONG && iz != type::LONG);
    return Ty;
  }
} } // end of namespace SUB_CONST_LONG_impl and c_compiler
