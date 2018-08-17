#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

std::vector<c_compiler::tac*> c_compiler::code;

c_compiler::ref::ref(const pointer_type* pt)
  : var(pt), m_result(pt->referenced_type()) {}

c_compiler::addrof::addrof(const type* T, var* ref, int offset)
  : var(T), m_ref(ref), m_offset(offset) {}

c_compiler::var* c_compiler::addrof::rvalue()
{
  if ( scope::current == &scope::root || expr::constant_flag )
    return this;
  else {
    var* t0 = new var(m_type);
    block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
    b ? b->m_vars.push_back(t0) : garbage.push_back(t0);
    code.push_back(new addr3ac(t0,m_ref));
    if ( m_offset ){
      var* t1 = new var(m_type);
      b ? b->m_vars.push_back(t1) : garbage.push_back(t1);
      var* off = integer::create(m_offset);
      code.push_back(new add3ac(t1,t0,off));
      return t1;
    }
    else
      return t0;
  }
}

c_compiler::genaddr::genaddr(const pointer_type* G, const type* T, var* ref, int offset)
  : var(G), generated(G,T), addrof(G,ref,offset), m_tmp(0)
{
  using namespace std;
  if ( m_offset ){
    m_tmp = new var(G);
    m_code.push_back(new addr3ac(m_tmp,m_ref));
    garbage.push_back(m_tmp);
    m_code.push_back(new add3ac(this,m_tmp,integer::create(offset)));
  }
  else
    m_code.push_back(new addr3ac(this,m_ref));
}

c_compiler::var* c_compiler::genaddr::rvalue()
{
  using namespace std;
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expr::constant_flag ){
    if ( !m_code.empty() ){
      vector<var*>& v = garbage;
      if ( m_tmp ){
        vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),m_tmp);
        assert(p != v.rend());
        vector<var*>::iterator q = p.base() - 1;
        q = v.erase(q);
        v.erase(q);
        b->m_vars.push_back(m_tmp);
      }
      else {
        vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
        assert(p != v.rend());
        v.erase(p.base()-1);
      }
      b->m_vars.push_back(this);
    }
  }
  if ( !expr::constant_flag ){
    copy(m_code.begin(),m_code.end(),back_inserter(code));
    m_code.clear();
  }
  if ( usr* u = m_ref->usr_cast() ){
    usr::flag& flag = u->m_flag;
    if ( flag & usr::REGISTER ){
      using namespace c_compiler::error::expr;
      address::implicit::specified_register(parse::position,u);
      flag = usr::flag(flag & ~usr::REGISTER);
    }
  }
  return this;
}

c_compiler::var* c_compiler::ref::rvalue()
{
  using namespace std;
  const type* T = m_result;
  if ( const pointer_type* G = T->ptr_gen() ){
    generated* ret = new generated(G,T);
    if ( scope::current->m_id == scope::BLOCK ){
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(ret);
    }
    else
      garbage.push_back(ret);
    code.push_back(new cast3ac(ret,this,G));
    return ret;
  }
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new invraddr3ac(ret,this));
  return ret;
}

c_compiler::var* c_compiler::var::promotion()
{
  const type* T = m_type;
  const type* P = T->promotion();
  return P != T ? cast(P) : this;
}

namespace c_compiler { namespace subscript_impl {
  var* size(const type*);
} } // end of namespace subscript_impl and c_compiler

c_compiler::var* c_compiler::var::subscripting(var* y)
{
  using namespace std;
  var* array = rvalue();
  var* index = y->rvalue();
  if ( !index->m_type->integer() )
    swap(array,index);
  if ( !index->m_type->integer() ){
    using namespace error::expr::subscripting;
    not_integer(parse::position,index);
    return array;
  }
  typedef const pointer_type PT;
  const type* T = array->m_type;
  T = T->unqualified();
  typedef const pointer_type PT;
  PT* pt = T->m_id == type::POINTER ? static_cast<PT*>(T) : 0;
  if ( !pt ){
    using namespace error::expr::subscripting;
    not_pointer(parse::position,array);
    return array;
  }
  pt = static_cast<PT*>(pt->complete_type());
  T = pt->referenced_type();
  var* size = subscript_impl::size(T);
  if ( !size ){
    using namespace error::expr::subscripting;
    not_object(parse::position,T);
    return array;
  }
  var* offset = size->mul(index);
  assert(offset->m_type->integer());
  return array->offref(T,offset);
}

c_compiler::var* c_compiler::genaddr::subscripting(var* y)
{
  var* index = y->rvalue();
  if ( !index->m_type->integer() ){
    using namespace error::expr::subscripting;
    not_integer(parse::position,index);
    return this;
  }
  {
    usr* u = static_cast<usr*>(m_ref);
    usr::flag& flag = u->m_flag;
    if ( flag & usr::REGISTER ){
      using namespace c_compiler::error::expr;
      address::implicit::specified_register(parse::position,u);
      flag = usr::flag(flag & ~usr::REGISTER);
    }
  }
  const type* T = m_ref->m_type;
  switch ( T->m_id ){
  case type::ARRAY:
    {
      typedef const array_type ARRAY;
      ARRAY* array = static_cast<ARRAY*>(T);
      T = array->element_type();
      break;
    }
  case type::VARRAY:
    {
      typedef const varray_type VARRAY;
      VARRAY* varray = static_cast<VARRAY*>(T);
      T = varray->element_type();
      break;
    }
  default:
    using namespace error::expr::subscripting;
    not_object(parse::position,T);
    return this;
  }
  var* size = subscript_impl::size(T);
  if ( !size ){
    using namespace error::expr::subscripting;
    not_object(parse::position,T);
    return this;
  }
  var* offset = size->mul(index);
  assert(offset->m_type->integer());
  return offref(T,offset);
}

c_compiler::var* c_compiler::constant<void*>::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = offset->value();
    unsigned char* p = reinterpret_cast<unsigned char*>(m_value);
    p += off;
    const pointer_type* pt = pointer_type::create(T);
    if (sizeof(void*) >= pt->size()) {
      void* pp = reinterpret_cast<void*>(p);
      var* ret = new refimm<void*>(pt, pp);
      garbage.push_back(ret);
      return ret;
    }
    else {
      __int64 lli = (__int64)p;
      var* ret = new refimm<__int64>(pt, lli);
      garbage.push_back(ret);
      return ret;
    }
  }
  return var::offref(T,offset);
}

c_compiler::var* c_compiler::subscript_impl::size(const type* T)
{
  using namespace std;
  if ( int size = T->size() )
    return integer::create(size);
  else
    return T->vsize();
}

c_compiler::var* c_compiler::genaddr::offref(const type* T, var* offset)
{
  typedef const pointer_type PT;
  PT* pt = pointer_type::create(T);
  if ( offset->isconstant() ){
    int off = m_offset + offset->value();
    var* ret = new refaddr(pt,m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( m_offset )
    offset = offset->add(integer::create(m_offset));
  var* ret = new refsomewhere(pt,m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

c_compiler::var* c_compiler::addrof::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = m_offset + offset->value();
    var* ret;
    if ( const pointer_type* G = T->ptr_gen() )
      ret = new genaddr(G,T,m_ref,off);
    else
      ret = new refaddr(pointer_type::create(T),m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( m_offset )
    offset = offset->add(integer::create(m_offset));
  if ( const pointer_type* G = T->ptr_gen() ){
    var* ret = new generated(G,T);
    block* b = (scope::current->m_id == scope::BLOCK) ? static_cast<block*>(scope::current) : 0;
    if ( b )
      b->m_vars.push_back(ret);
    else
      garbage.push_back(ret);
    code.push_back(new addr3ac(ret,m_ref));
    code.push_back(new add3ac(ret,ret,offset));
    return ret;
  }
  var* ret = new refsomewhere(pointer_type::create(T),m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

c_compiler::var* c_compiler::var::offref(const type* T, var* offset)
{
  using namespace std;
  if ( m_type->scalar() ){
    ref* ret = new ref(pointer_type::create(T));
    if ( scope::current->m_id == scope::BLOCK ){
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(ret);
    }
    else
      garbage.push_back(ret);
    if ( offset->isconstant() && !offset->value() )
      code.push_back(new assign3ac(ret,this));
    else
      code.push_back(new add3ac(ret,this,offset));
    return ret;
  }
  if ( offset->isconstant() ){
    int off = offset->value();
    var* ret = new refaddr(pointer_type::create(T),this,off);
    garbage.push_back(ret);
    return ret;
  }
  var* ret = new refsomewhere(pointer_type::create(T),this,offset);
  garbage.push_back(ret);
  return ret;
}

c_compiler::var* c_compiler::refaddr::offref(const type* T, var* offset)
{
  if ( offset->isconstant() ){
    int off = m_addrof.m_offset + offset->value();
    var* ret = new refaddr(pointer_type::create(T),m_addrof.m_ref,off);
    garbage.push_back(ret);
    return ret;
  }
  if ( int n = m_addrof.m_offset )
    offset = offset->add(integer::create(n));
  var* ret = new refsomewhere(pointer_type::create(T),m_addrof.m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

c_compiler::var* c_compiler::refsomewhere::offref(const type* T, var* offset)
{
  offset = offset->add(m_offset);
  var* ret = new refsomewhere(pointer_type::create(T),m_ref,offset);
  garbage.push_back(ret);
  return ret;
}

namespace c_compiler { namespace call_impl {
  var* common(const func_type*, var*, std::vector<var*>*, function_definition::static_inline::info* = 0);
} } // end of namespace call_impl and c_compiler

c_compiler::var* c_compiler::var::call(std::vector<var*>* arg)
{
  using namespace std;
  var* func = rvalue();
  const type* T = func->m_type;
  if ( T->backpatch() ){
    usr* u = static_cast<usr*>(func);
    usr::flag& flag = u->m_flag;
    flag = usr::flag(flag | usr::FUNCTION);
    warning::expr::call::implicit(u);
    vector<const type*> param;
    param.push_back(ellipsis_type::create());
    u->m_type = T = func_type::create(int_type::create(),param,true);
    string name = u->m_name;
    scope::current->m_usrs[name].push_back(u);
  }
  else {
    typedef const pointer_type PT;
    T = T->unqualified();
    if ( T->m_id == type::POINTER ){
      PT* pt = static_cast<PT*>(T);
      T = pt->referenced_type();
    }
  }
  if ( T->m_id == type::FUNC ){
    typedef const func_type FUNC;
    FUNC* ft = static_cast<FUNC*>(T);
    return call_impl::common(ft,func,arg);
  }
  else {
    using namespace error::expr::call;
    not_function(parse::position,func);
    return func;
  }
}

namespace c_compiler { namespace call_impl {
  std::pair<int,int> num_of_range(const std::vector<const type*>&);
  struct convert {
    const std::vector<const type*>& m_param;
    var* m_func;
    int m_counter;
    convert(const std::vector<const type*>& param, var* func)
      : m_param(param), m_func(func), m_counter(-1) {}
    var* operator()(var*);
  };
  tac* gen_param(var*);
  namespace inlined {
    tac* assign_param(var*, var*);
  } // end of namespace inlined
} } // end of namespace call_impl and c_compiler


namespace c_compiler { namespace function_definition { namespace static_inline { namespace symtab {
  std::map<var*, var*> table;
} } } }  // end of namespace symtab, static_inline, function_definition and c_compiler

namespace c_compiler { namespace function_definition { namespace static_inline { namespace dup {
  struct patch {
    std::map<goto3ac*,goto3ac*> m_goto;
    std::map<to3ac*,to3ac*> m_to;
  };
  tac* filter(tac*, patch*);
  int spatch(std::pair<goto3ac*,goto3ac*>, std::map<to3ac*,to3ac*>*);
  int tpatch(std::pair<to3ac*,to3ac*>, std::map<goto3ac*,goto3ac*>*);
} } } } // end of namespace dup, static_inline, function_definition and c_compiler

c_compiler::tac* c_compiler::function_definition::static_inline::dup::filter(tac* ptr, patch* patch)
{
  using namespace std;
  tac* ret = ptr->new3ac();
  if ( ret->x ){
    map<var*, var*>::const_iterator p = symtab::table.find(ret->x);
    if ( p != symtab::table.end() )
      ret->x = p->second;
  }
  if ( ret->y ){
    map<var*, var*>::const_iterator p = symtab::table.find(ret->y);
    if ( p != symtab::table.end() )
      ret->y = p->second;
  }
  if ( ret->z ){
    map<var*, var*>::const_iterator p = symtab::table.find(ret->z);
    if ( p != symtab::table.end() )
      ret->z = p->second;
  }

  tac::id_t id = ptr->id;
  switch ( id ){
  case tac::GOTO:
    {
      goto3ac* go = static_cast<goto3ac*>(ptr);
      return patch->m_goto[go] = static_cast<goto3ac*>(ret);
    }
  case tac::TO:
    {
      to3ac* to = static_cast<to3ac*>(ptr);
      return patch->m_to[to] = static_cast<to3ac*>(ret);
    }
  default:
    return ret;
  }
}

int
c_compiler::function_definition::static_inline::dup::spatch(std::pair<goto3ac*,goto3ac*> x, std::map<to3ac*,to3ac*>* y)
{
  using namespace std;
  goto3ac* org = x.first;
  map<to3ac*,to3ac*>::const_iterator p = y->find(org->m_to);
  assert(p != y->end());
  goto3ac* _new = x.second;
  _new->m_to = p->second;
  return 0;
}

namespace c_compiler { namespace function_definition { namespace static_inline { namespace dup {
  goto3ac* helper(goto3ac*, std::map<goto3ac*,goto3ac*>*);
} } } } // end of namespace dup, static_inline, function_definition and c_compiler

int
c_compiler::function_definition::static_inline::dup::tpatch(std::pair<to3ac*,to3ac*> x, std::map<goto3ac*,goto3ac*>* y)
{
  using namespace std;
  to3ac* org = x.first;
  const vector<goto3ac*>& u = org->m_goto;
  to3ac* _new = x.second;
  vector<goto3ac*>& v = _new->m_goto;
  transform(u.begin(),u.end(),v.begin(),bind2nd(ptr_fun(helper),y));
  return 0;
}

c_compiler::goto3ac*
c_compiler::function_definition::static_inline::dup::helper(goto3ac* x, std::map<goto3ac*,goto3ac*>* y)
{
  using namespace std;
  map<goto3ac*,goto3ac*>::const_iterator p = y->find(x);
  assert(p != y->end());
  return p->second;
}

namespace c_compiler { namespace function_definition { namespace static_inline { namespace expand {
  block* create(const param_scope*);
  struct arg {
    std::vector<tac*>* m_expand;
    std::vector<goto3ac*>* m_returns;
    var* m_ret;
    dup::patch* m_patch;
  };
  int conv(tac*, arg*);
} } } }  // end of namespace expand, static_inline, function_definition and c_compiler

void c_compiler::function_definition::static_inline::expand::action(info* info)
{
  using namespace std;
  fundef* def = info->m_fundef;
  info->m_param = create(def->m_param);
  const type* T = def->m_usr->m_type;
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(T);
  T = func->return_type();
  if ( !T->compatible(void_type::create()) ){
    info->m_ret = new var(T);
    info->m_ret->m_scope = info->m_param;
    vector<scope*>& c = info->m_param->m_children;
    assert(!c.empty());
    block* b = static_cast<block*>(c[0]);
    b->m_vars.push_back(info->m_ret);
  }
  const vector<tac*>& v = info->m_code;
  vector<goto3ac*> returns;
  dup::patch patch;
  arg arg = { &info->m_expanded, &returns, info->m_ret, &patch };
  for_each(v.begin(),v.end(),bind2nd(ptr_fun(conv),&arg));
  map<goto3ac*,goto3ac*>& s = patch.m_goto;
  map<to3ac*,to3ac*>& t = patch.m_to;
  for_each(s.begin(),s.end(),bind2nd(ptr_fun(dup::spatch),&t));
  for_each(t.begin(),t.end(),bind2nd(ptr_fun(dup::tpatch),&s));
  to3ac* to = new to3ac;
  info->m_expanded.push_back(to);
  for_each(returns.begin(),returns.end(),bind2nd(ptr_fun(update),to));
  copy(returns.begin(),returns.end(),back_inserter(to->m_goto));
}

namespace c_compiler { namespace function_definition { namespace static_inline { namespace expand {
  usr* new_usr(usr*, scope*);
  scope* new_block(scope*, scope*);
} } } }  // end of namespace expand, static_inline, function_definition and c_compiler

c_compiler::block*
c_compiler::function_definition::static_inline::expand::create(const param_scope* param)
{
  using namespace std;
  symtab::table.clear();
  block* ret = new block;
  ret->m_parent = scope::current;
  const vector<usr*>& o = param->m_order;
  vector<var*>& v = ret->m_vars;
  transform(o.begin(),o.end(),back_inserter(v),bind2nd(ptr_fun(new_usr),ret));
  const vector<scope*>& c = param->m_children;
  transform(c.begin(),c.end(),back_inserter(ret->m_children),bind2nd(ptr_fun(new_block),ret));
  return ret;
}

c_compiler::usr*
c_compiler::function_definition::static_inline::expand::new_usr(usr* u, scope* s)
{
  usr* ret = new usr(*u);
  ret->m_scope = s;
  symtab::table[u] = ret;
  return ret;
}

namespace c_compiler { namespace function_definition { namespace static_inline { namespace expand {
  var* new_var(var*, scope*);
} } } }  // end of namespace expand, static_inline, function_definition and c_compiler

c_compiler::scope*
c_compiler::function_definition::static_inline::expand::new_block(scope* ptr, scope* parent)
{
  using namespace std;
  block* ret = new block;
  ret->m_parent = parent;
  assert(ptr->m_id == scope::BLOCK);
  block* b = static_cast<block*>(ptr);
  const map<string, vector<usr*> >& u = b->m_usrs;
  map<string, vector<usr*> >& d = ret->m_usrs;
  typedef map<string, vector<usr*> >::const_iterator IT;
  for ( IT p = u.begin() ; p != u.end() ; ++p ){
    string name = p->first;
    const vector<usr*>& v = p->second;
    transform(v.begin(),v.end(),back_inserter(d[name]),bind2nd(ptr_fun(new_usr),ret));
  }
  const vector<var*>& v = b->m_vars;
  transform(v.begin(),v.end(),back_inserter(ret->m_vars),bind2nd(ptr_fun(new_var),ret));
  const vector<scope*>& c = b->m_children;
  transform(c.begin(),c.end(),back_inserter(ret->m_children),bind2nd(ptr_fun(new_block),ret));
  return ret;
}

c_compiler::var*
c_compiler::function_definition::static_inline::expand::new_var(var* v, scope* s)
{
  var* ret = new var(*v);
  ret->m_scope = s;
  return symtab::table[v] = ret;
}

int c_compiler::function_definition::static_inline::expand::conv(tac* tac, arg* pa)
{
  using namespace std;
  if ( tac->id == tac::RETURN ){
    if ( var* y = tac->y ){
      map<var*,var*>::const_iterator p = symtab::table.find(y);
      if ( p != symtab::table.end() )
        y = p->second;
      pa->m_expand->push_back(new assign3ac(pa->m_ret,y));
    }
    goto3ac* go = new goto3ac;
    pa->m_returns->push_back(go);
    pa->m_expand->push_back(go);
  }
  else
    pa->m_expand->push_back(dup::filter(tac,pa->m_patch));
  return 0;
}

c_compiler::var* c_compiler::genaddr::call(std::vector<var*>* arg)
{
  using namespace std;
  const type* T = m_ref->m_type;
  if ( T->m_id != type::FUNC ){
    using namespace error::expr::call;
    not_function(parse::position,m_ref);
    return rvalue();
  }
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(T);
  mark();
  usr* u = static_cast<usr*>(m_ref);
  usr::flag flag = u->m_flag;
  if ( flag & usr::INLINE ){
    function_definition::static_inline::skipped_t::const_iterator p =
      function_definition::static_inline::skipped.find(u);
    if ( p != function_definition::static_inline::skipped.end() )
      return call_impl::common(func,u,arg,p->second);
#if 0
    else if ( !function_definition::Inline::resolve::flag ){
      vector<var*>* tmp = arg ? new vector<var*>(*arg) : 0;
      int n = code.size();
      var* v = call_impl::common(func,u,tmp);
      if ( error::counter ){
        delete arg;
        return v;
      }
      int m = code.size();
      for_each(code.begin()+n,code.begin()+m,deleter<tac>());
      code.resize(n);
      const type* T = func->return_type();
      assign3ac* point = new assign3ac(0,0);
      function_definition::Inline::after* ret =
        new function_definition::Inline::after(T,u,arg,point);
      code.push_back(point);
      block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
      if ( b && !T->compatible(void_type::create()) )
        b->m_vars.push_back(ret);
      return ret;
    }
	else {
		assert(0);
		function_definition::static_inline::todo::lists.insert(name);
  }
#endif
  }
  return call_impl::common(func,m_ref,arg);
}

namespace c_compiler {
	bool take_va(const func_type* ft)
	{
		using namespace std;
		const vector<const type*>& param = ft->param();
		if (param.empty())
			return false;
		const type* T = param.back();
		return T->m_id == type::ELLIPSIS;
	}
} // end of namespace c_compiler

c_compiler::var*
c_compiler::call_impl::common(const func_type* ft, var* func, std::vector<var*>* arg,
                              function_definition::static_inline::info* info)
{
  using namespace std;
  auto_ptr<vector<var*> > sweeper(arg);
  const vector<const type*>& param = ft->param();
  int n = arg ? arg->size() : 0;
  pair<int,int> m = call_impl::num_of_range(param);
  if ( n < m.first ){
    using namespace error::expr::call;
    num_of_arg(parse::position,func,n,m.first);
    info = 0;
  }
  else if ( m.second < n ){
    using namespace error::expr::call;
    num_of_arg(parse::position,func,n,m.second);
    n = m.second;
    info = 0;
  }
  vector<var*> conved;
  if ( arg ){
    // arg->end() ‚Å‚È‚­ arg->begin() + n ‚Å‚ ‚é‚Ì‚Í m.second < n ‚Ìê‡ n ‚ªXV‚³‚ê‚é‚©‚ç
    transform(arg->begin(),arg->begin()+n,back_inserter(conved),call_impl::convert(param,func));
  }
  bool tva = info ? take_va(static_cast<const func_type*>(info->m_fundef->m_usr->m_type)) : false;
  if ( info && !tva ){
    function_definition::static_inline::expand::action(info);
    if ( scope::current->m_id == scope::BLOCK ){
      block* b = static_cast<block*>(scope::current);
      vector<scope*>& c = b->m_children;
      c.push_back(info->m_param);
    }
    const vector<var*>& order = info->m_param->m_vars;
    transform(order.begin(),order.end(),conved.begin(),back_inserter(code),call_impl::inlined::assign_param);
    vector<tac*>& v = info->m_expanded;
    copy(v.begin(),v.end(),back_inserter(code));
    v.clear();
  }
  else {
    transform(conved.begin(),conved.end(),back_inserter(code),call_impl::gen_param);
  }
  const type* T = ft->return_type();
  T = T->complete_type();
  var* x = new var(T);
  if ( T->compatible(void_type::create()) ){
    if ( !info || tva )
      code.push_back(new call3ac(0,func));
    garbage.push_back(x);
    return x;
  }
  if ( !T->size() ){
    using namespace error::expr::call;
    not_object(parse::position,func);
    x->m_type = int_type::create();
  }
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  if ( info && !tva )
    code.push_back(new assign3ac(x,info->m_ret));
  else
    code.push_back(new call3ac(x,func));
  return x;
}


bool c_compiler::function_definition::Inline::after::expand(std::string name, std::vector<tac*>& dst)
{
  using namespace std;
  if ( m_func->m_name != name )
    return false;
  scope* org = scope::current;
  scope::current = m_scope;
  genaddr genaddr(0,0,m_func,0);
  var* ret = genaddr.call(m_arg);
  scope::current = org;
  bool b = m_type->compatible(void_type::create());
  if ( !b )
    code.push_back(new assign3ac(this,ret));
  vector<tac*>::iterator p = find(dst.begin(),dst.end(),m_point);
  assert(p != dst.end());
  delete *p;
  p = dst.erase(p);
  dst.insert(p,code.begin(),code.end());
  code.clear();
  if ( b )
    delete this;
  return true;
}

c_compiler::function_definition::Inline::after::after(const type* T, usr* func, std::vector<var*>* arg, tac* point)
  : var(T), m_func(func), m_arg(arg), m_scope(scope::current), m_point(point)
{
  lists.push_back(this);
}

std::vector<c_compiler::function_definition::Inline::after*>
c_compiler::function_definition::Inline::after::lists;

void c_compiler::genaddr::mark()
{
#if 0
  using namespace std;
  usr* u = static_cast<usr*>(m_ref);
  usr::flag flag = u->m_flag;
  if ( !(flag & usr::INLINE) && (flag & usr::STATIC)
       || u->m_scope->m_id == scope::BLOCK ){
    string name = u->m_name;
    map<string, usr*>::const_iterator p =
      function_definition::table.find(name);
    if ( p != function_definition::table.end() ){
      const function_definition::static_inline::skipped_t& m =
        function_definition::static_inline::skipped;
	  if (m.find(name) != m.end()) {
		  usr::flag cflag = fundef::current->m_usr->m_flag;
		  usr::flag mask = usr::flag(usr::INLINE | usr::STATIC);
		  if (cflag & mask) {
			  string curr = fundef::current->m_usr->m_name;
			  function_definition::static_inline::todo::chain[curr].insert(name);
		  }
		  else
			  function_definition::static_inline::todo::lists.insert(name);
	  }
    }
    else {
		using namespace function_definition::static_inline;
		just_refed::table[name] = new just_refed::info(parse::position, flag, fundef::current->m_usr);
	}
  }
#endif
}

std::pair<int,int> c_compiler::call_impl::num_of_range(const std::vector<const type*>& param)
{
  using namespace std;
  const type* T = param.back();
  if ( T->m_id == type::ELLIPSIS )
    return make_pair(param.size()-1,INT_MAX);
  if ( T->compatible(void_type::create()) )
    return make_pair(0,0);
  else
    return make_pair(param.size(),param.size());
}

c_compiler::var* c_compiler::call_impl::convert::operator()(var* arg)
{
  using namespace std;
  arg = arg->rvalue();
  if ( ++m_counter == m_param.size() )
    --m_counter;
  const type* T = m_param[m_counter];
  T = T->complete_type();
  if ( T->m_id == type::ELLIPSIS ){
    const type* type = arg->m_type;
    if ( type->compatible(type->varg()) )
      return arg;
    T = type->varg();
  }
  T = T->unqualified();
  bool discard = false;
  T = expr::assign_impl::valid(T,arg,&discard);
  if ( !T ){
    using namespace error::expr::call;
    mismatch_argument(parse::position,m_counter,discard,m_func);
    return arg;
  }
  return arg->cast(T);
}

c_compiler::tac* c_compiler::call_impl::gen_param(var* y)
{
  return new param3ac(y);
}

c_compiler::tac* c_compiler::call_impl::inlined::assign_param(var* x, var* y)
{
  return new assign3ac(x,y);
}

c_compiler::var* c_compiler::var::member(usr* id, bool dot)
{
  using namespace std;
  auto_ptr<usr> sweeper(id);
  const type* T = result_type();
  int cvr = 0;
  T = T->unqualified(dot ? &cvr : 0);
  typedef const pointer_type PT;
  if ( !dot ){
    if ( T->m_id == type::POINTER ){
      PT* pt = static_cast<PT*>(T);
      T = pt->referenced_type();
      T = T->unqualified(&cvr);
    }
    else {
      using namespace error::expr::member;
      not_pointer(id,this);
      return this;
    }
  }
  T = T->complete_type();
  if ( T->m_id != type::RECORD ){
    using namespace error::expr::member;
    not_record(id,this);
    return this;
  }
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(T);
  pair<int, usr*> off = rec->offset(id->m_name);
  int offset = off.first;
  if ( offset < 0 ){
    using namespace error::expr::member;
    not_member(id,rec,this);
    return this;
  }
  usr* member = off.second;
  T = member->m_type;
  if ( member->m_flag & usr::BIT_FIELD ){
    int pos = rec->position(member);
    typedef const bit_field_type BF;
    BF* bf = static_cast<BF*>(T);
    T = bf->integer_type();
    PT* pt = pointer_type::create(T);
    int bit = bf->bit();
    var* ret = new refbit(pt,this,offset,member,pos,bit,dot);
    garbage.push_back(ret);
    return ret;
  }
  T = T->qualified(cvr);
  return offref(T,integer::create(offset));
}

c_compiler::var* c_compiler::var::ppmm(bool plus, bool post)
{
  using namespace std;
  if ( !lvalue() ){
    using namespace error::expr::ppmm;
    not_lvalue(parse::position,plus,this);
  }
  const type* T = m_type;
  T = T->promotion();
  if ( !T->scalar() ){
    using namespace error::expr::ppmm;
    not_scalar(parse::position,plus,this);
    return this;
  }
  if ( !T->modifiable() ){
    using namespace error::expr::ppmm;
    not_modifiable(parse::position,plus,this);
    return this;
  }
  var* one = integer::create(1);
  if ( T->arithmetic() )
    one = one->cast(T);
  else {
    const type* TT = T->unqualified();
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(TT);
    TT = pt->referenced_type();
    TT = TT->complete_type();
    if ( !(one = TT->vsize()) ){
      int n = TT->size();
      if ( !n ){
        using namespace error::expr::ppmm;
        invalid_pointer(parse::position,plus,pt);
        n = 1;
      }
      one = integer::create(n);
    }
  }
  var* ret = new var(T);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  b ? b->m_vars.push_back(ret) : garbage.push_back(ret);
  if ( post ){
    if ( T->compatible(m_type) ){
      code.push_back(new assign3ac(ret,this));
      if ( plus )
        code.push_back(new add3ac(this,this,one));
      else
        code.push_back(new sub3ac(this,this,one));
    }
    else {
      code.push_back(new cast3ac(ret,this,T));
      var* tmp = new var(T);
      b ? b->m_vars.push_back(tmp) : garbage.push_back(tmp);
      code.push_back(new assign3ac(tmp,ret));
      if ( plus )
        code.push_back(new add3ac(tmp,tmp,one));
      else
        code.push_back(new sub3ac(tmp,tmp,one));
      code.push_back(new cast3ac(this,tmp,m_type));
    }
  }
  else {
    if ( T->compatible(m_type) ){
      if ( plus )
        code.push_back(new add3ac(this,this,one));
      else
        code.push_back(new sub3ac(this,this,one));
      code.push_back(new assign3ac(ret,this));
    }
    else {
      code.push_back(new cast3ac(ret,this,T));
      if ( plus )
        code.push_back(new add3ac(ret,ret,one));
      else
        code.push_back(new sub3ac(ret,ret,one));
      code.push_back(new cast3ac(this,ret,m_type));
    }
  }
  return ret;
}

c_compiler::var* c_compiler::ref::ppmm(bool plus, bool post)
{
  using namespace std;
  const type* T = m_result;
  T = T->promotion();
  if ( !T->scalar() ){
    using namespace error::expr::ppmm;
    not_scalar(parse::position,plus,this);
    return this;
  }
  if ( !T->modifiable() ){
    using namespace error::expr::ppmm;
    not_modifiable(parse::position,plus,this);
    return this;
  }
  var* one = integer::create(1);
  if ( T->arithmetic() )
    one = one->cast(T);
  else {
    const type* TT = T->unqualified();
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(TT);
    TT = pt->referenced_type();
    int n = TT->size();
    if ( !n ){
      using namespace error::expr::ppmm;
      invalid_pointer(parse::position,plus,pt);
      n = 1;
    }
    one = integer::create(n);
  }
  var* ret = rvalue();
  ret = ret->promotion();
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( post ){
    var* tmp = new var(T);
    b ? b->m_vars.push_back(tmp) : garbage.push_back(tmp);
    if ( plus )
      code.push_back(new add3ac(tmp,ret,one));
    else
      code.push_back(new sub3ac(tmp,ret,one));
    assign(tmp);
  }
  else {
    if ( plus )
      code.push_back(new add3ac(ret,ret,one));
    else
      code.push_back(new sub3ac(ret,ret,one));
    assign(ret);
  }
  return ret;
}

c_compiler::var* c_compiler::generated::ppmm(bool plus, bool post)
{
  using namespace error::expr::ppmm;
  not_modifiable_lvalue(parse::position,plus,m_org);
  return this;
}

namespace c_compiler { namespace literal {
  struct compound : usr {
    compound(std::string name, const type* T, usr::flag flag, const file_t& file)
      : usr(name,T,flag,file) {}
  };
} } // end of namespace literal and c_compiler

c_compiler::var* c_compiler::expr::compound_literal(const type* T, parse::initializer_list* il)
{
  using namespace std;
  using namespace decl;
  typedef const pointer_type PT;
  typedef const array_type ARRAY;
  auto_ptr<parse::initializer_list> sweeper(il);
  if ( scope::current != &scope::root ){
    string name = new_name(".compound");
    usr* ret = new literal::compound(name,T,usr::NONE,parse::position);
    if ( scope::current->m_id == scope::BLOCK )
      scope::current->m_usrs[name].push_back(ret);
    else
      garbage.push_back(ret);
    map<int,var*> v;
    initializer::argument::dst = ret;
    initializer::argument arg(T,v,0,0,0,0,0,il->size());
    for_each(il->begin(),il->end(),initializer_list::eval(arg));
    initializer::fill_zero(arg);
    for_each(v.begin(),v.end(),bind1st(ptr_fun(gen_loff),ret));
    if ( T->m_id == type::ARRAY ){
      ARRAY* array = static_cast<ARRAY*>(T);
      if ( !array->dim() ){
        T = array->element_type();
        int n = T->size();
        int dim = arg.off_max / n;
        ret->m_type = T = array_type::create(T,dim);
      }
      PT* G = array->ptr_gen();
      var* gen = new genaddr(G,T,ret,0);
      garbage.push_back(gen);
      return gen;
    }
    else
      return ret;
  }
  else {
    string name = new_name(".compound");
    with_initial* ret = new with_initial(name,T,parse::position);
    initializer::argument::dst = ret;
    initializer::argument arg(T,ret->m_value,0,0,0,0,0,il->size());
    for_each(il->begin(),il->end(),c_compiler::initializer_list::eval(arg));
    initializer::fill_zero(arg);
    scope::current->m_usrs[name].push_back(ret);
    if ( T->m_id == type::ARRAY ){
      ARRAY* array = static_cast<ARRAY*>(T);
      if ( !array->dim() ){
        T = array->element_type();
        int n = T->size();
        int dim = arg.off_max / n;
        ret->m_type = T = array_type::create(T,dim);
      }
      PT* G = T->ptr_gen();
      var* gen = new genaddr(G,T,ret,0);
      garbage.push_back(gen);
      return gen;
    }
    else
      return ret;
  }
}

c_compiler::var* c_compiler::expr::size(const type* T)
{
  using namespace std;
  T = T->complete_type();
  if ( var* size = T->vsize() )
    return size;
  else {
    unsigned int n = T->size();
    if ( !n ){
      using namespace error::expr::size;
      invalid(parse::position,T);
      n = 1;
    }
    return integer::create(n);
  }
}

c_compiler::var* c_compiler::var::address()
{
  using namespace error::expr::address;
  not_lvalue(parse::position);
  return this;
}

c_compiler::var* c_compiler::usr::address()
{
  if ( !lvalue() ){
    using namespace error::expr::address;
    not_lvalue(parse::position);
  }
  if ( m_flag & usr::REGISTER ){
    using namespace error::expr::address;
    specified_register(parse::position,this);
    m_flag = usr::flag(m_flag & ~usr::REGISTER);
  }
  typedef const pointer_type PT;
  PT* pt = pointer_type::create(m_type);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expr::constant_flag ){
    var* x = new var(pt);
    b->m_vars.push_back(x);
    code.push_back(new addr3ac(x,this));
    return x;
  }
  else {
    var* ret = new addrof(pt,this,0);
    garbage.push_back(ret);
    return ret;
  }
}

c_compiler::var* c_compiler::genaddr::address()
{
  const type* T = m_ref->m_type;
  if ( T->m_id == type::FUNC )
    mark();
  if ( usr* u = m_ref->usr_cast() ){
    usr::flag& flag = u->m_flag;
    if ( flag & usr::REGISTER ){
      using namespace error::expr::address;
      specified_register(parse::position,u);
      flag = usr::flag(flag & ~usr::REGISTER);
    }
  }
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expr::constant_flag ){
    var* ret = new var(pointer_type::create(m_ref->m_type));
    b->m_vars.push_back(ret);
    code.push_back(new addr3ac(ret,m_ref));
    return ret;
  }
  else {
    var* ret = new addrof(pointer_type::create(m_ref->m_type),m_ref,0);
    garbage.push_back(ret);
    return ret;
  }
}

c_compiler::var* c_compiler::ref::address()
{
  var* ret = new var(m_type);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new assign3ac(ret,this));
  return ret;
}

c_compiler::var* c_compiler::refaddr::address()
{
  using namespace std;
  if ( !lvalue() ){
    using namespace error::expr::address;
    not_lvalue(parse::position);
    return this;
  }
  if ( usr* u = m_addrof.m_ref->usr_cast() ){
    usr::flag& flag = u->m_flag;
    if ( flag & usr::REGISTER ){
      using namespace error::expr::address;
      specified_register(parse::position,u);
      flag = usr::flag(flag & ~usr::REGISTER);
    }
  }
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  if ( b && !expr::constant_flag ){
    const type* T = m_type;
    var* t0 = new var(T);
    b->m_vars.push_back(t0);
    code.push_back(new addr3ac(t0,m_addrof.m_ref));
    if ( m_addrof.m_offset ){
      var* t1 = new var(T);
      b->m_vars.push_back(t1);
      usr* off = integer::create(m_addrof.m_offset);
      code.push_back(new add3ac(t1,t0,off));
      return t1;
    }
    else
      return t0;
  }
  else {
    typedef const pointer_type PT;
    PT* pt = static_cast<PT*>(m_type);
    usr* u = static_cast<usr*>(m_addrof.m_ref);
    var* ret = new addrof(pt,u,m_addrof.m_offset);
    garbage.push_back(ret);
    return ret;
  }
}

c_compiler::var* c_compiler::refbit::address()
{
  using namespace error::expr::address;
  bit_field(parse::position,m_member);
  return refaddr::address();
}

c_compiler::var* c_compiler::refsomewhere::address()
{
  using namespace std;
  var* ret = new var(m_type);
  if ( scope::current->m_id == scope::BLOCK ){
    vector<var*>& v = garbage;
    vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
    assert(p != v.rend());
    v.erase(p.base()-1);
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(this);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new addr3ac(this,m_ref));
  code.push_back(new add3ac(ret,this,m_offset));
  return ret;
}

c_compiler::var* c_compiler::var::indirection()
{
  using namespace std;
  decl::check(this);
  var* y = rvalue();
  const type* T = y->m_type;
  T = T->unqualified();
  typedef const pointer_type PT;
  PT* pt = T->m_id == type::POINTER ? static_cast<PT*>(T) : 0;
  if ( !pt ){
    using namespace error::expr::indirection;
    not_pointer(parse::position);
    pt = pointer_type::create(T);
  }
  ref* x = new ref(pt);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(x);
  }
  else
    garbage.push_back(x);
  code.push_back(new assign3ac(x,y));
  return x;
}

c_compiler::var* c_compiler::addrof::indirection()
{
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(m_type);
  var* ret = new refaddr(pt,m_ref,m_offset);
  garbage.push_back(ret);
  return ret;
}

c_compiler::var* c_compiler::genaddr::indirection()
{
  if ( usr* u = m_ref->usr_cast() ){
    usr::flag& flag = u->m_flag;
    if ( flag & usr::REGISTER ){
      using namespace error::expr::address::implicit;
      specified_register(parse::position,u);
      flag = usr::flag(flag & ~usr::REGISTER);
    }
  }
  const type* T = m_ref->m_type;
  if ( T->m_id == type::FUNC ){
    mark();
    return this;
  }
  return addrof::indirection();
}

c_compiler::var* c_compiler::constant<void*>::indirection()
{
  typedef const pointer_type PT;
  PT* pt = static_cast<PT*>(m_type);
  var* ret = new refimm<void*>(pt,m_value);
  garbage.push_back(ret);
  return ret;
}

c_compiler::var* c_compiler::var::_not()
{
  using namespace std;
  var* expr = rvalue();
  const type* T = expr->m_type;
  if ( !T->scalar() ){
    using namespace error::expr::unary;
    invalid(parse::position,'!',T);
  }
  expr = expr->promotion();
  usr* zero = integer::create(0);
  usr* one = integer::create(1);
  var01* ret = new var01(int_type::create());
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  var* tmp = zero->cast(expr->m_type);
  goto3ac* goto1 = new goto3ac(goto3ac::NE,expr,tmp);
  code.push_back(goto1);
  ret->m_one = code.size();
  code.push_back(new assign3ac(ret,one));
  goto3ac* goto2 = new goto3ac;
  code.push_back(goto2);
  to3ac* to1 = new to3ac;
  to1->m_goto.push_back(goto1);
  code.push_back(to1);
  goto1->m_to = to1;
  ret->m_zero = code.size();
  code.push_back(new assign3ac(ret,zero));
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  to2->m_goto.push_back(goto2);
  goto2->m_to = to2;
  return ret;
}

c_compiler::var* c_compiler::var::plus()
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if ( !T->arithmetic() ){
    using namespace error::expr::unary;
    invalid(parse::position,'+',T);
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new assign3ac(ret,expr));
  return ret;
}

c_compiler::var* c_compiler::var::minus()
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if ( !T->arithmetic() ){
    using namespace error::expr::unary;
    invalid(parse::position,'-',T);
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new uminus3ac(ret,expr));
  return ret;
}

namespace c_compiler { namespace constant_impl {
	template<class T> var* minus(constant<T>* p)
	{
		return integer::create(-p->m_value);
	}
#ifdef _MSC_VER
	template<> var* minus(constant<unsigned int>* p)
	{
		return integer::create((unsigned int)(-(int)p->m_value));
	}
	template<> var* minus(constant<unsigned long int>* p)
	{
		return integer::create((unsigned long int)(-(long int)p->m_value));
	}
	template<> var* minus(constant<unsigned __int64>* p)
	{
		return integer::create((unsigned __int64)(-(__int64)p->m_value));
	}
#endif // _MSC_VER
} } // end of constant_impl and c_compiler

namespace c_compiler {
  template<> var* constant<char>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<signed char>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<unsigned char>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<short int>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<unsigned short int>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<int>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<unsigned int>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<long int>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<unsigned long int>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<__int64>::minus()
  { return constant_impl::minus(this); }
  template<> var* constant<unsigned __int64>::minus()
  { return constant_impl::minus(this); }
} // end of namespace c_compiler

c_compiler::var* c_compiler::constant<float>::minus()
{
  return floating::create(-m_value);
}

c_compiler::var* c_compiler::constant<double>::minus()
{
  return floating::create(-m_value);
}

c_compiler::var* c_compiler::constant<long double>::minus()
{
  if ( generator::long_double ){
    int sz = long_double_type::create()->size();
    unsigned char* p = new unsigned char[sz];
    (*generator::long_double->minus)(p,b);
    return floating::create(p);
  }
  else
    return floating::create(-m_value);
}

c_compiler::var* c_compiler::var::tilde()
{
  var* expr = rvalue();
  const type* T = expr->m_type;
  if ( !T->arithmetic() || !T->integer() ){
    using namespace error::expr::unary;
    invalid(parse::position,'~',T);
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  code.push_back(new tilde3ac(ret,expr));
  return ret;
}

namespace c_compiler { namespace constant_impl {
  template<class T> var* tilde(constant<T>* y)
  {
    return integer::create(~y->m_value);
  }
} } // end of namespace constant_impl and c_compiler

namespace c_compiler {
  template<> var* constant<char>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<signed char>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<unsigned char>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<short int>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<unsigned short int>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<int>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<unsigned int>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<long int>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<unsigned long int>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<__int64>::tilde()
  { return constant_impl::tilde(this); }
  template<> var* constant<unsigned __int64>::tilde()
  { return constant_impl::tilde(this); }
} // end of namespace c_compiler

c_compiler::var* c_compiler::refaddr::rvalue()
{
  const type* T = m_result;
  if ( const pointer_type* G = T->ptr_gen() ){
    genaddr* tmp = new genaddr(G,T,m_addrof.m_ref,m_addrof.m_offset);
    garbage.push_back(tmp);
    return tmp->rvalue();
  }
  var* ret = new var(T);
  if ( scope::current->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(scope::current);
    b->m_vars.push_back(ret);
  }
  else
    garbage.push_back(ret);
  var* off = integer::create(m_addrof.m_offset);
  code.push_back(new roff3ac(ret,m_addrof.m_ref,off));
  return ret;
}

namespace c_compiler {
	const type* unsigned_type(const type* T)
	{
		if (T->compatible(char_type::create()))
			return uchar_type::create();
		if (T->compatible(schar_type::create()))
			return uchar_type::create();
		if (T->compatible(short_type::create()))
			return ushort_type::create();
		if (T->compatible(int_type::create()))
			return uint_type::create();
		if (T->compatible(long_type::create()))
			return ulong_type::create();
		if (T->compatible(long_long_type::create()))
			return ulong_long_type::create();
		return 0;
	}
}  // end of namespace c_compiler

c_compiler::var* c_compiler::refbit::rvalue()
{
  using namespace std;
  const type* T = m_result;
  var* ret = new var(T);
  block* b = scope::current->m_id == scope::BLOCK ? static_cast<block*>(scope::current) : 0;
  assert(b);
  b->m_vars.push_back(ret);
  if ( m_dot ){
    var* offset = integer::create(m_addrof.m_offset);
    code.push_back(new roff3ac(ret,m_addrof.m_ref,offset));
  }
  else {
    var* ptr = m_addrof.m_ref;
    if ( m_addrof.m_offset ){
      ptr = new var(m_type);
      b->m_vars.push_back(ptr);
      usr* off = integer::create(m_addrof.m_offset);
      code.push_back(new add3ac(ptr,m_addrof.m_ref,off));
    }
    code.push_back(new invraddr3ac(ret,ptr));
  }
  if ( m_position ){
    if (T->_signed()){
      const type* U = unsigned_type(T);
      var* tmp = new var(U);
      b->m_vars.push_back(tmp);
      code.push_back(new cast3ac(tmp,ret,U));
      ret = tmp;
    }
    usr* pos = integer::create(m_position);
    var* tmp = new var(T);
    b->m_vars.push_back(tmp);
    code.push_back(new rsh3ac(tmp,ret,pos));
    ret = tmp;
  }
  usr* m = mask(m_bit);
  var* tmp = new var(T);
  b->m_vars.push_back(tmp);
  code.push_back(new and3ac(tmp,ret,m));
  ret = tmp;
  if ( T->_signed() ){
    usr* m = integer::create(1 << (m_bit-1));
    var* tmp = new var(T);
    b->m_vars.push_back(tmp);
    code.push_back(new and3ac(tmp,ret,m));
    usr* zero = integer::create(0);
    goto3ac* go = new goto3ac(goto3ac::EQ,tmp,zero);
    code.push_back(go);
    {
      usr* ext = integer::create((~0 << m_bit));
      code.push_back(new or3ac(ret,ret,ext));
    }
    to3ac* to = new to3ac;
    code.push_back(to);
    to->m_goto.push_back(go);
    go->m_to = to;
  }
  return ret;
}

c_compiler::usr* c_compiler::refbit::mask(int n)
{
  return integer::create(~(~0 << n));
}

c_compiler::usr* c_compiler::refbit::mask(int n, int pos)
{
  using namespace std;
  n = ~(~0 << n);
  n <<= pos;
  n = ~n;
  return integer::create(n);
}

c_compiler::var* c_compiler::refsomewhere::rvalue()
{
  block* b = ( scope::current->m_id == scope::BLOCK ) ? static_cast<block*>(scope::current) : 0;
  const type* T = m_result;
  if ( const pointer_type* G = T->ptr_gen() ){
    var* tmp = new var(G);
    var* ret = new generated(G,T);
    if ( b ) {
      b->m_vars.push_back(tmp);
      b->m_vars.push_back(ret);
    }
    else {
      garbage.push_back(tmp);
      garbage.push_back(ret);
    }
    code.push_back(new addr3ac(tmp,m_ref));
    code.push_back(new add3ac(ret,tmp,m_offset));
    return ret;
  }
  var* ret = new var(T);
  if ( b )
    b->m_vars.push_back(ret);
  else
    garbage.push_back(ret);
  code.push_back(new roff3ac(ret,m_ref,m_offset));
  return ret;
}

void c_compiler::decl::check(var* v)
{
	using namespace std;
	using namespace error;
	const type* T = v->m_type;
	if (T->m_id == type::BACKPATCH) {
		usr* u = static_cast<usr*>(v);
		string name = u->m_name;
		undeclared(u->m_file, name);
		map<string, vector<usr*> >& usrs = scope::current->m_usrs;
		u->m_type = int_type::create();
		usrs[name].push_back(u);
	}
}

c_compiler::var* c_compiler::var::size(int n)
{
  using namespace std;
  decl::check(this);
  var* expr;
  {
    struct sweeper {
      int n;
      sweeper(int nn) : n(nn) {}
      ~sweeper()
      {
        using namespace std;
        using namespace c_compiler;
        for_each(code.begin()+n,code.end(),deleter<tac>());
        code.resize(n);
      }
    } sweeper(n);
    expr = rvalue();
  }
  return expr->size();
}

c_compiler::var* c_compiler::var::size()
{
  return expr::size(m_type);
}

c_compiler::var* c_compiler::generated::size()
{
  return expr::size(m_org);
}

namespace c_compiler { namespace constant_impl {
  template<class T> var* size(constant<T>* y, int n)
  {
    using namespace std;
    if ( y->m_type->compatible(char_type::create()) ){
      unsigned int n = int_type::create()->size();
      return integer::create(n);
    }
    else
      return y->var::size(n);
  }
} } // end of namespace constant_impl and c_compiler

namespace c_compiler {
  template<> var* constant<char>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<signed char>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<unsigned char>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<short int>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<unsigned short int>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<int>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<unsigned int>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<long int>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<unsigned long int>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<__int64>::size(int n)
  { return constant_impl::size(this,n); }
  template<> var* constant<unsigned __int64>::size(int n)
  { return constant_impl::size(this,n); }
} // end of namespace c_compiler

c_compiler::var* c_compiler::refbit::size(int n)
{
  using namespace error::expr::size;
  bit_field(parse::position,m_member);
  return refaddr::size(n);
}

c_compiler::var* c_compiler::genaddr::size(int n)
{
  return generated::size();
}

namespace c_compiler {  
  template<> var* constant<char>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<signed char>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<unsigned char>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<short int>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<unsigned short int>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<int>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<unsigned int>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<long int>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<unsigned long int>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<__int64>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
  template<> var* constant<unsigned __int64>::_not()
  { return zero() ? integer::create(1) : integer::create(0); }
} // end of namespace c_compiler

c_compiler::var* c_compiler::constant<float>::_not()
{ return zero() ? integer::create(1) : integer::create(0); }
c_compiler::var* c_compiler::constant<double>::_not()
{ return zero() ? integer::create(1) : integer::create(0); }
c_compiler::var* c_compiler::constant<long double>::_not()
{ return zero() ? integer::create(1) : integer::create(0); }
c_compiler::var* c_compiler::constant<void*>::_not()
{ return zero() ? integer::create(1) : integer::create(0); }

bool c_compiler::constant<long double>::zero()
{
  return generator::long_double ?
    (*generator::long_double->zero)(b) : m_value == 0.0L;
}
