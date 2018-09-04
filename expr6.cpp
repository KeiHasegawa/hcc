#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

c_compiler::var* c_compiler::var::assign(var* expr)
{
  using namespace error::expr::assign;
  not_lvalue(parse::position);
  return this;
}

c_compiler::var* c_compiler::usr::assign(var* op)
{
  m_type = m_type->complete_type();
  const type* T = m_type;
  if ( !T->modifiable() ){
    using namespace error::expr::assign;
    not_modifiable(parse::position,this);
  }
  var* y = op->rvalue();
  y->m_type = y->m_type->complete_type();
  bool discard = false;
  T = expr::assign_impl::valid(T,y,&discard);
  if ( !T ){
    using namespace error::expr::assign;
    invalid(parse::position,this,discard);
    T = int_type::create();
  }
  y = y->cast(T);
  code.push_back(new assign3ac(this,y));
  if ( !y->isconstant() )
    return y;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,y));
  return x;
}

c_compiler::var* c_compiler::generated::assign(var*)
{
  using namespace error::expr::assign;
  not_modifiable_lvalue(parse::position,m_org);
  return this;
}

c_compiler::var* c_compiler::ref::assign(var* op)
{
  m_result = m_result->complete_type();
  const type* T = m_result;
  if ( !T->modifiable() ){
    using namespace error::expr::assign;
    not_modifiable(parse::position,0);
  }
  bool discard = false;
  var* y = op->rvalue();
  T = expr::assign_impl::valid(T,y,&discard);
  if ( !T ){
    using namespace error::expr::assign;
    invalid(parse::position,0,discard);
    T = int_type::create();
  }
  y = y->cast(T);
  code.push_back(new invladdr3ac(this,y));
  if ( !y->isconstant() )
    return y;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,y));
  return x;
}

c_compiler::var* c_compiler::refaddr::assign(var* op)
{
  using namespace std;
  if ( !lvalue() ){
    using namespace error::expr::assign;
    not_lvalue(parse::position);
    return this;
  }
  m_result = m_result->complete_type();
  const type* T = m_result;
  if ( !T->modifiable() ){
    using namespace error::expr::assign;
    not_modifiable(parse::position,0);
  }
  bool discard = false;
  var* z = op->rvalue();
  T = expr::assign_impl::valid(T,z,&discard);
  if ( !T ){
    using namespace error::expr::assign;
    invalid(parse::position,0,discard);
    T = int_type::create();
  }
  z = z->cast(T);
  var* y = integer::create(m_addrof.m_offset);
  code.push_back(new loff3ac(m_addrof.m_ref,y,z));
  if ( !z->isconstant() )
    return z;
  var* x = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,z));
  return x;
}

c_compiler::var* c_compiler::refbit::assign(var* op)
{
  using namespace std;
  const type* T = m_result;
  var* x = new var(T);
  var* y = op->rvalue();
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  assert(b);
  b->m_vars.push_back(x);
  usr* u = mask(m_bit);
  code.push_back(new and3ac(x,y,u));
  var* t0 = 0;
  if ( m_position ){
    t0 = new var(T);
    b->m_vars.push_back(t0);
    usr* pos = integer::create(m_position);
    code.push_back(new lsh3ac(t0,x,pos));
  }
  var* t1 = new var(T);
  b->m_vars.push_back(t1);
  if ( m_dot ){
    var* off = integer::create(m_addrof.m_offset);
    code.push_back(new roff3ac(t1,m_addrof.m_ref,off));
  }
  else {
    var* ptr = m_addrof.m_ref;
    if ( m_addrof.m_offset ){
      ptr = new var(m_type);
      b->m_vars.push_back(ptr);
      usr* off = integer::create(m_addrof.m_offset);
      code.push_back(new add3ac(ptr,m_addrof.m_ref,off));
    }
    code.push_back(new invraddr3ac(t1,ptr));
  }
  usr* v = mask(m_bit,m_position);
  code.push_back(new and3ac(t1,t1,v));
  code.push_back(new or3ac(t1,t1,t0 ? t0 : x));
  if ( m_dot ){
    var* off = integer::create(m_addrof.m_offset);
    code.push_back(new loff3ac(m_addrof.m_ref,off,t1));
  }
  else {
    var* ptr = m_addrof.m_ref;
    if ( m_addrof.m_offset ){
      ptr = new var(m_type);
      b->m_vars.push_back(ptr);
      usr* off = integer::create(m_addrof.m_offset);
      code.push_back(new add3ac(ptr,m_addrof.m_ref,off));
    }
    code.push_back(new invladdr3ac(ptr,t1));
  }
  return x;
}

c_compiler::var* c_compiler::refsomewhere::assign(var* op)
{
  op = op->rvalue();
  const type* T = m_result;
  bool discard = false;
  T = expr::assign_impl::valid(T,op,&discard);
  if ( !T ){
    using namespace error::expr::assign;
    invalid(parse::position,0,discard);
    T = int_type::create();
  }
  op = op->cast(T);
  code.push_back(new loff3ac(m_ref,m_offset,op));
  if ( !op->isconstant() )
    return op;
  var* ret = new var(m_result);
  block* b = ( scope::current->m_id == scope::BLOCK ) ? static_cast<block*>(scope::current) : 0;
  if ( b )
    b->m_vars.push_back(ret);
  else
    garbage.push_back(ret);
  code.push_back(new assign3ac(ret,op));
  return ret;
}

const c_compiler::type* c_compiler::expr::assign_impl::valid(const type* T, var* src, bool* discard)
{
  using namespace std;
  decl::check(src);

  const type* xx = T;
  const type* yy = src->m_type;
  xx = xx->unqualified();
  yy = yy->unqualified();

  if ( xx->arithmetic() && yy->arithmetic() )
    return xx;

  if ( xx->m_id == type::RECORD ){
    typedef const record_type REC;
    REC* rec = static_cast<REC*>(xx);
    if ( rec->compatible(yy) )
      return rec;
    return 0;
  }

  typedef const pointer_type PT;
  if ( xx->m_id == type::POINTER ){
    PT* lp = static_cast<PT*>(xx);
    if ( yy->m_id == type::POINTER ){
      PT* rp = static_cast<PT*>(yy);
      if ( lp->compatible(rp) ){
        if ( !discard || lp->include_cvr(rp) )
          return lp;
        else {
          *discard = true;
          return 0;
        }
      }
      const type* vp = pointer_type::create(void_type::create());
      if ( vp->compatible(lp) ){
        if ( lp->include_cvr(rp) )
          return lp;
        else {
                  if (discard)
            *discard = true;
          return 0;
        }
      }
      if ( vp->compatible(rp) ){
        if ( lp->include_cvr(rp) )
          return lp;
        else {
                  if (discard)
            *discard = true;
          return 0;
        }
      }
    }
  }

  if ( xx->m_id == type::POINTER ){
    if ( yy->integer() && src->zero() )
      return xx;
  }

  return 0;
}

c_compiler::var* c_compiler::var::comma(var* y)
{
  using namespace std;
  decl::check(this);
  decl::check(y);

  rvalue();
  var* v = y->rvalue();
  if ( !v->lvalue() )
    return v;
  const type* T = v->m_type;
  T = T->complete_type();
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  assert(!T->compatible(void_type::create()));
  code.push_back(new assign3ac(ret,v));
  return ret;
}

bool c_compiler::expr::constant_flag;

std::string c_compiler::new_name(std::string head)
{
  using namespace std;
  ostringstream os;
  static int cnt;
  os << head << cnt++;
  return os.str();
}

int c_compiler::decl::declarator::array_impl::point;
