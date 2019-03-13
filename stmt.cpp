#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

namespace c_compiler { namespace stmt {
  namespace label {
    std::map<std::string, data_t> data;
    std::map<std::string, std::vector<used_t> > used;
    std::vector<usr*> vm;
  } // end of namespace label
  namespace goto_impl {
    bool outside(const scope*, const scope*);
    bool cmp(const usr*, const scope*);
    int decide(label::used_t, to3ac*);
  } // end of namespace goto_impl
} } // end of namespace stmt and c_compiler

void c_compiler::stmt::label::action(usr* u)
{
  using namespace std;
  using namespace goto_impl;
  if (genaddr* ga = u->genaddr_cast()) {
    vector<var*>::reverse_iterator p = find(garbage.rbegin(), garbage.rend(), ga);
    assert(p != garbage.rend());
    var* v = ga->m_ref;
    u = v->usr_cast();
    assert(u);
    usr* tmp = new usr(*u);
    u = tmp;
    garbage.erase(p.base()-1);
  }
  else if (u->m_type->m_id != type::BACKPATCH) {
    usr* tmp = new usr(*u);
    u = tmp;
  }
  auto_ptr<usr> sweeper(u);
  string name = u->m_name;
  {
    map<string,data_t>::const_iterator p = data.find(name);
    if ( p != data.end() ){
      using namespace error::stmt::label;
      multiple(name,p->second.m_file,parse::position);
    }
  }
  to3ac* to = new to3ac;
  code.push_back(to);
  data[name] = data_t(to,parse::position,scope::current);
  map<string, vector<used_t> >::iterator p = used.find(name);
  if ( p != used.end() ){
    const vector<used_t>& v = p->second;
    for_each(v.begin(),v.end(),bind2nd(ptr_fun(decide),to));
    used.erase(p);
  }
}

int c_compiler::stmt::goto_impl::decide(label::used_t u, to3ac* to)
{
  using namespace std;
  if ( outside(u.m_scope,scope::current) ){
    const vector<usr*>& v = label::vm;
    vector<usr*>::const_iterator p =
      find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp),scope::current));
    if ( p != v.end() ){
      using namespace error::stmt::_goto;
      invalid(u.m_file,parse::position,*p);
    }
  }
  goto3ac* go = u.m_goto;
  go->m_to = to;
  to->m_goto.push_back(go);
  return 0;
}

bool c_compiler::stmt::goto_impl::outside(const scope* from, const scope* to)
{
  if ( const scope* parent = to->m_parent ){
    if ( from == parent )
      return true;
    else
      return outside(from,parent);
  }
  else
    return false;
}

bool c_compiler::stmt::goto_impl::cmp(const usr* u, const scope* s)
{
  return u->m_scope == s;
}

void c_compiler::stmt::expr(var* expr)
{
  using namespace std;
  expr = expr->rvalue();
  decl::check(expr);
  const type* T = expr->m_type;
  if ( tag* tag = T->get_tag() ){
    if ( !tag->m_types.second ){
      using namespace error::stmt::expr;
      incomplete_type(parse::position);
    }
  }
}

void c_compiler::stmt::if_expr(var* expr)
{
  expr->if_expr();
}

namespace c_compiler { namespace stmt {
  namespace if_impl {
    struct info {
      var* m_expr;
      goto3ac* m_goto1;
      goto3ac* m_goto2;
      int m_point1;
      int m_point2;
      info(var* expr, goto3ac* goto1)
        : m_expr(expr), m_goto1(goto1), m_goto2(0), m_point1(-1), m_point2(-1) {}
      template<class T> info(constant<T>* c)
        : m_expr(c), m_goto1(0), m_goto2(0), m_point1(code.size()), m_point2(-1) {}
      info(var01* expr)
        : m_expr(expr), m_goto1(0), m_goto2(0), m_point1(code.size()), m_point2(-1) {}
    };
    std::vector<info> ifs;
  }
} }

void c_compiler::var::if_expr()
{
  using namespace std;
  using namespace stmt;
  using namespace if_impl;
  var* expr = rvalue();
  decl::check(expr);
  const type* T = expr->m_type;

  if ( !T->scalar() ){
    using namespace error::stmt::_if;
    not_scalar(parse::position,expr);
    T = int_type::create();
  }
  T = T->promotion();
  expr = expr->cast(T);
  var* zero = integer::create(0);
  zero = zero->cast(T);
  goto3ac* go = new goto3ac(goto3ac::EQ,expr,zero);
  ifs.push_back(info(this,go));
  code.push_back(go);
}

namespace c_compiler { namespace stmt { namespace if_impl {
  template<class T> void expr(constant<T>* ptr)
  {
    ifs.push_back(info(ptr));
  }
} } } // end of namespace if_impl, stmt and _compiler

namespace c_compiler {
  template<>
  void constant<char>::if_expr(){ stmt::if_impl::expr(this); }
  template<>
  void constant<signed char>::if_expr(){ stmt::if_impl::expr(this); }
  template<>
  void constant<unsigned char>::if_expr(){ stmt::if_impl::expr(this); }
  template<>
  void constant<short int>::if_expr(){ stmt::if_impl::expr(this); }
  template<>
  void constant<unsigned short int>::if_expr(){ stmt::if_impl::expr(this); }

  void constant<int>::if_expr(){ stmt::if_impl::expr(this); }

  void constant<unsigned int>::if_expr(){ stmt::if_impl::expr(this); }

  void constant<long int>::if_expr(){ stmt::if_impl::expr(this); }

  void constant<unsigned long int>::if_expr(){ stmt::if_impl::expr(this); }

  void constant<__int64>::if_expr(){ stmt::if_impl::expr(this); }

  void constant<unsigned __int64>::if_expr(){ stmt::if_impl::expr(this); }
} // end of namespace c_compiler

void c_compiler::constant<float>::if_expr(){ stmt::if_impl::expr(this); }
void c_compiler::constant<double>::if_expr(){ stmt::if_impl::expr(this); }
void c_compiler::constant<long double>::if_expr(){ stmt::if_impl::expr(this); }
void c_compiler::constant<void*>::if_expr(){ stmt::if_impl::expr(this); }

void c_compiler::var01::if_expr()
{
  using namespace stmt;
  using namespace if_impl;
  ifs.push_back(info(this));
}

void c_compiler::stmt::else_action()
{
  using namespace if_impl;
  ifs.back().m_expr->else_action();
}

void c_compiler::var::else_action()
{
  using namespace std;
  using namespace stmt;
  using namespace if_impl;
  info& info = ifs.back();
  code.push_back(info.m_goto2 = new goto3ac);
  goto3ac* goto1 = info.m_goto1;
  to3ac* to = new to3ac;
  code.push_back(to);
  to->m_goto.push_back(goto1);
  goto1->m_to = to;
}

namespace c_compiler { namespace stmt { namespace if_impl {
  template<class T> void _else(constant<T>* ptr)
  {
    info& info = ifs.back();
    assert(info.m_expr == ptr);
    info.m_point2 = code.size();
  }
} } } // end of namespace if_impl, stmt and _compiler

namespace c_compiler {
  template<>
  void constant<char>::else_action(){ stmt::if_impl::_else(this); }
  template<>
  void constant<signed char>::else_action(){ stmt::if_impl::_else(this); }
  template<>
  void constant<unsigned char>::else_action(){ stmt::if_impl::_else(this); }
  template<>
  void constant<short int>::else_action(){ stmt::if_impl::_else(this); }
  template<>
  void constant<unsigned short int>::else_action(){ stmt::if_impl::_else(this); }

  void constant<int>::else_action(){ stmt::if_impl::_else(this); }

  void constant<unsigned int>::else_action(){ stmt::if_impl::_else(this); }

  void constant<long int>::else_action(){ stmt::if_impl::_else(this); }

  void constant<unsigned long int>::else_action(){ stmt::if_impl::_else(this); }

  void constant<__int64>::else_action(){ stmt::if_impl::_else(this); }

  void constant<unsigned __int64>::else_action(){ stmt::if_impl::_else(this); }
} // end of namespace c_compiler

void c_compiler::constant<float>::else_action(){ stmt::if_impl::_else(this); }
void c_compiler::constant<double>::else_action(){ stmt::if_impl::_else(this); }
void c_compiler::constant<long double>::else_action(){ stmt::if_impl::_else(this); }
void c_compiler::constant<void*>::else_action(){ stmt::if_impl::_else(this); }

void c_compiler::var01::else_action()
{
  using namespace stmt;
  using namespace if_impl;
  info& x = ifs.back();
  x.m_point2 = code.size();
}

void c_compiler::stmt::end_if()
{
  using namespace if_impl;
  ifs.back().m_expr->end_if();
}

void c_compiler::var::end_if()
{
  using namespace std;
  using namespace stmt;
  using namespace if_impl;
  info& info = ifs.back();
  to3ac* to = new to3ac;
  code.push_back(to);
  if ( goto3ac* go2 = info.m_goto2 ){
    go2->m_to = to;
    to->m_goto.push_back(go2);
  }
  else {
    goto3ac* go1 = info.m_goto1;
    go1->m_to = to;
    to->m_goto.push_back(go1);
  }
  ifs.pop_back();
}

namespace c_compiler { namespace stmt { namespace if_impl {
  template<class T> void end(constant<T>* ptr)
  {
    using namespace std;
    info& info = ifs.back();
    assert(ptr == info.m_expr);
    if ( info.m_point2 < 0 ){
      if ( ptr->zero() ){
        int n = info.m_point1;
        for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
        code.resize(n);
      }
    }
    else {
      if ( ptr->zero() ){
        int n = info.m_point1;
        int m = info.m_point2;
        for_each(code.begin()+n,code.begin()+m,[](tac* p){ delete p; });
        vector<tac*>::iterator p = code.begin() + n;
        vector<tac*>::iterator q = code.begin() + m;
        code.erase(p,q);
      }
      else {
        int n = info.m_point2;
        for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
        code.resize(n);
      }
    }
    ifs.pop_back();
  }
} } } // end of namespace if_impl, stmt and _compiler

namespace c_compiler {
  template<>
  void constant<char>::end_if(){ stmt::if_impl::end(this); }
  template<>
  void constant<signed char>::end_if(){ stmt::if_impl::end(this); }
  template<>
  void constant<unsigned char>::end_if(){ stmt::if_impl::end(this); }
  template<>
  void constant<short int>::end_if(){ stmt::if_impl::end(this); }
  template<>
  void constant<unsigned short int>::end_if(){ stmt::if_impl::end(this); }

  void constant<int>::end_if(){ stmt::if_impl::end(this); }

  void constant<unsigned int>::end_if(){ stmt::if_impl::end(this); }

  void constant<long int>::end_if(){ stmt::if_impl::end(this); }

  void constant<unsigned long int>::end_if(){ stmt::if_impl::end(this); }

  void constant<__int64>::end_if(){ stmt::if_impl::end(this); }

  void constant<unsigned __int64>::end_if(){ stmt::if_impl::end(this); }
} // end of namespace c_compiler

void c_compiler::constant<float>::end_if(){ stmt::if_impl::end(this); }
void c_compiler::constant<double>::end_if(){ stmt::if_impl::end(this); }
void c_compiler::constant<long double>::end_if(){ stmt::if_impl::end(this); }
void c_compiler::constant<void*>::end_if(){ stmt::if_impl::end(this); }

void c_compiler::var01::sweep(std::vector<tac*>::iterator it)
{
  it -= 2;
  delete *it;
  it = code.erase(it);
  ++it;
  delete *it;
  code.erase(it);
}

void c_compiler::var01::sweep()
{
  using namespace std;
  block* b = static_cast<block*>(m_scope);
  vector<var*>& v = b->m_vars;
  vector<var*>::reverse_iterator q = find(v.rbegin(),v.rend(),this);
  assert(q != v.rend());
  v.erase(q.base()-1);
  garbage.push_back(this);
}

void c_compiler::var01::end_if()
{
  using namespace std;
  using namespace stmt;
  using namespace if_impl;
  sweep();
  info& info = ifs.back();
  int n = info.m_point1;
  int m = info.m_point2;
  vector<tac*> stmt1, stmt2;
  if ( m < 0 ){
    copy(code.begin()+n,code.end(),back_inserter(stmt1));
    code.resize(n);
  }
  else {
    copy(code.begin()+m,code.end(),back_inserter(stmt2));
    copy(code.begin()+n,code.begin()+m,back_inserter(stmt1));
    code.resize(n);
  }
  if ( m_one < m_zero ){
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt1.begin(),stmt1.end());
    vector<tac*>::iterator q = code.begin() + m_zero + stmt1.size() - 1;
    delete *q;
    q = code.erase(q);
    if ( m < 0 )
      sweep(q);
    else
      code.insert(q,stmt2.begin(),stmt2.end());
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_zero;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt2.begin(),stmt2.end());
    vector<tac*>::iterator q = code.begin() + m_one + stmt2.size() - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,stmt1.begin(),stmt1.end());
  }
  ifs.pop_back();
}

namespace c_compiler { namespace stmt {
  namespace break_impl {
    struct info : std::vector<goto3ac*> {};
    std::vector<info*> outer;
  }
  namespace switch_impl {
    struct info : break_impl::info {
      var* m_expr;
      goto3ac* m_start;
      struct case_t {
        var* m_label;
        to3ac* m_to;
        file_t m_file;
        scope* m_scope;
        case_t(var* v, to3ac* to, const file_t& file, scope* s)
          : m_label(v), m_to(to), m_file(file), m_scope(s) {}
        static bool cmp(case_t x, var* y)
        {
	  var* tmp = expr::binary(EQUAL_MK, x.m_label, y);
	  assert(tmp->isconstant());
	  return tmp->value() != 0;
	}
      };
      std::vector<case_t> m_cases;
      struct default_t {
        to3ac* m_to;
        file_t m_file;
        scope* m_scope;
        default_t(to3ac* to, const file_t& file, scope* s)
          : m_to(to), m_file(file), m_scope(s) {}
      };
      default_t m_default;
      info(var* expr, goto3ac* start)
        : m_expr(expr), m_start(start), m_default(0,file_t(),0) {}
    };
    std::vector<info*> switchs;
  }
} } // end of namespace stmt and c_compiler

void c_compiler::stmt::switch_expr(var* expr)
{
  using namespace std;
  using namespace switch_impl;
  expr = expr->rvalue();
  decl::check(expr);
  const type* T = expr->m_type;

  if ( !T->integer() ){
    using namespace error::stmt::_switch;
    not_integer(parse::position,expr);
    expr->m_type = int_type::create();
  }
  goto3ac* start = new goto3ac;
  info* ptr = new info(expr, start);
  switchs.push_back(ptr);
  break_impl::outer.push_back(ptr);
  code.push_back(start);
}

void c_compiler::stmt::_case(var* expr)
{
  using namespace std;
  using namespace switch_impl;
  expr = expr->rvalue();
  decl::check(expr);

  if ( !expr->isconstant() ){
    using namespace error::stmt::_case;
    not_constant(parse::position,expr);
    return;
  }

  const type* T = expr->m_type;
  if ( !T->integer() ){
    using namespace error::stmt::_case;
    not_integer(parse::position,expr);
    return;
  }
  if ( switchs.empty() ){
    using namespace error::stmt::_case;
    no_switch(parse::position);
  }
  else {
    info* ptr = switchs.back();
    vector<info::case_t>& v = ptr->m_cases;
    vector<info::case_t>::const_iterator p =
      find_if(v.begin(),v.end(),bind2nd(ptr_fun(info::case_t::cmp),expr));
    if ( p != v.end() ){
      using namespace error::stmt::_case;
      duplicate(parse::position,p->m_file);
    }
    else {
      to3ac* to = new to3ac;
      code.push_back(to);
      v.push_back(info::case_t(expr,to,parse::position,scope::current));
    }
  }
}

void c_compiler::stmt::_default()
{
  using namespace std;
  using namespace switch_impl;
  if ( switchs.empty() ){
    using namespace error::stmt::_default;
    no_switch(parse::position);
  }
  else {
    info* ptr = switchs.back();
    if ( !ptr->m_default.m_to ){
      to3ac* to = new to3ac;
      code.push_back(to);
	  ptr->m_default = info::default_t(to,parse::position,scope::current);
    }
    else {
      using namespace error::stmt::_default;
      multiple(parse::position, ptr->m_default.m_file);
    }
  }
}

namespace c_compiler { namespace stmt { namespace switch_impl {
    int gencode(var*, info::case_t);
} } } // end of namespace switch_impl, stmt and c_compiler

void c_compiler::stmt::end_switch()
{
  using namespace std;
  using namespace switch_impl;
  break_impl::outer.pop_back();
  assert(!switchs.empty());
  info* ptr = switchs.back();
  goto3ac* end = new goto3ac;
  code.push_back(end);
  goto3ac* start = ptr->m_start;
  to3ac* to1 = new to3ac;
  code.push_back(to1);
  to1->m_goto.push_back(start);
  start->m_to = to1;
  var* expr = ptr->m_expr;
  vector<info::case_t>& cases = ptr->m_cases;
  for_each(cases.begin(),cases.end(),bind1st(ptr_fun(gencode),expr));
  if ( to3ac* to = ptr->m_default.m_to ){
    using namespace goto_impl;
    if ( outside(scope::current, ptr->m_default.m_scope) ){
      const vector<usr*>& v = label::vm;
      vector<usr*>::const_iterator p =
        find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp), ptr->m_default.m_scope));
      if ( p != v.end() ){
        using namespace error::stmt::_switch;
        invalid(false, ptr->m_default.m_file,*p);
      }
    }
    goto3ac* go = new goto3ac;
    code.push_back(go);
    go->m_to = to;
    to->m_goto.push_back(go);
  }
  to3ac* to2 = new to3ac;
  code.push_back(to2);
  for_each(ptr->begin(), ptr->end(),bind2nd(ptr_fun(update),to2));
  copy(ptr->begin(), ptr->end(),back_inserter(to2->m_goto));
  to2->m_goto.push_back(end);
  end->m_to = to2;
  switchs.pop_back();
  delete ptr;
}

int c_compiler::stmt::switch_impl::gencode(var* x, info::case_t _case)
{
  using namespace std;
  using namespace goto_impl;
  if ( outside(scope::current,_case.m_scope) ){
    const vector<usr*>& v = label::vm;
    vector<usr*>::const_iterator p =
      find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp),_case.m_scope));
    if ( p != v.end() ){
      using namespace error::stmt::_switch;
      invalid(true,_case.m_file,*p);
    }
  }
  var* y = _case.m_label;
  conversion::arithmetic(&x,&y);
  goto3ac* go = new goto3ac(goto3ac::EQ,x,y);
  code.push_back(go);
  to3ac* to = _case.m_to;
  go->m_to = to;
  to->m_goto.push_back(go);
  return 0;
}

namespace c_compiler { namespace stmt {
  namespace continue_impl {
    struct info : std::vector<goto3ac*> {};
    std::vector<info*> outer;
  }
  namespace while_impl {
    struct info : break_impl::info, continue_impl::info {
      var* m_expr;
      to3ac* m_to;
      goto3ac* m_goto;
      int m_point;
      info(var* expr, to3ac* to, goto3ac* go)
        : m_expr(expr), m_to(to), m_goto(go), m_point(-1) {}
      template<class T> info(constant<T>* c, to3ac* to)
        : m_expr(c), m_to(to), m_goto(0), m_point(code.size()) {}
      info(var01* expr, to3ac* to)
        : m_expr(expr), m_to(to), m_goto(0), m_point(code.size()) {}
    };
    std::vector<info*> whiles;
  }
} } // end of namespace stmt and c_compiler

void c_compiler::stmt::while_expr(to3ac* to, var* expr)
{
  expr->while_expr(to);
}

void c_compiler::var::while_expr(to3ac* to)
{
  using namespace stmt;
  using namespace while_impl;
  var* expr = rvalue();
  expr = expr->promotion();
  const type* T = expr->m_type;
  if ( !T->scalar() ){
    using namespace error::stmt::_while;
    not_scalar(parse::position);
    T = expr->m_type = int_type::create();
  }
  var* zero = integer::create(0);
  zero = zero->cast(T);
  goto3ac* go = new goto3ac(goto3ac::EQ,expr,zero);
  code.push_back(go);
  info* ptr = new info(this,to,go);
  whiles.push_back(ptr);
  break_impl::outer.push_back(ptr);
  continue_impl::outer.push_back(ptr);
}

namespace c_compiler { namespace stmt { namespace while_impl {
  template<class T> void expr(constant<T>* cons, to3ac* to)
  {
    info* ptr = new info(cons,to);
    whiles.push_back(ptr);
    break_impl::outer.push_back(ptr);
    continue_impl::outer.push_back(ptr);
  }
} } } // end of namespace while_impl, stmt and c_compiler

namespace c_compiler {
  template<>
  void constant<char>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
  template<>
  void constant<signed char>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
  template<>
  void constant<unsigned char>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
  template<>
  void constant<short int>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
  template<>
  void constant<unsigned short int>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }

  void constant<int>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }

  void constant<unsigned int>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }

  void constant<long int>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }

  void constant<unsigned long int>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }

  void constant<__int64>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }

  void constant<unsigned __int64>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
} // end of namespace c_compiler

void c_compiler::constant<float>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
void c_compiler::constant<double>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
void c_compiler::constant<long double>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }
void c_compiler::constant<void*>::while_expr(to3ac* to){ stmt::while_impl::expr(this,to); }

void c_compiler::var01::while_expr(to3ac* to)
{
  using namespace stmt;
  using namespace while_impl;
  info* ptr = new info(this,to);
  whiles.push_back(ptr);
  break_impl::outer.push_back(ptr);
  continue_impl::outer.push_back(ptr);
}

void c_compiler::stmt::end_while()
{
  using namespace while_impl;
  info* ptr = whiles.back();
  ptr->m_expr->end_while();
}

void c_compiler::var::end_while()
{
  using namespace std;
  using namespace stmt;
  using namespace while_impl;
  info* ptr = whiles.back();
  goto3ac* goto1 = new goto3ac;
  code.push_back(goto1);
  to3ac* to_continue = ptr->m_to;
  goto1->m_to = to_continue;
  to_continue->m_goto.push_back(goto1);
  {
    const continue_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
    copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
    continue_impl::outer.pop_back();
  }
  goto3ac* goto2 = ptr->m_goto;
  to3ac* to_break = new to3ac;
  code.push_back(to_break);
  {
    const break_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
    copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
    break_impl::outer.pop_back();
  }
  to_break->m_goto.push_back(goto2);
  goto2->m_to = to_break;
  whiles.pop_back();
  delete ptr;
}

namespace c_compiler { namespace stmt { namespace while_impl {
  template<class T> void end(constant<T>* cons)
  {
    using namespace std;
    info* ptr = whiles.back();
    assert(ptr->m_expr == cons);
    if ( cons->zero() ){
      int n = ptr->m_point;
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
      code.resize(n);
    }
    else {
      goto3ac* goto1 = new goto3ac;
      code.push_back(goto1);
      to3ac* to1 = ptr->m_to;
      goto1->m_to = to1;
      to1->m_goto.push_back(goto1);
      {
        to3ac* to_continue = to1;
        const continue_impl::info& p = *ptr;
        for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
        copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
        continue_impl::outer.pop_back();
      }
      {
        to3ac* to_break = new to3ac;
        code.push_back(to_break);
        const break_impl::info& p = *ptr;
        for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
        copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
        break_impl::outer.pop_back();
      }
    }
    whiles.pop_back();
  }
} } } // end of namespace while_impl, stmt and c_compiler

namespace c_compiler {
  template<>
  void constant<char>::end_while(){ stmt::while_impl::end(this); }
  template<>
  void constant<signed char>::end_while(){ stmt::while_impl::end(this); }
  template<>
  void constant<unsigned char>::end_while(){ stmt::while_impl::end(this); }
  template<>
  void constant<short int>::end_while(){ stmt::while_impl::end(this); }
  template<>
  void constant<unsigned short int>::end_while(){ stmt::while_impl::end(this); }

  void constant<int>::end_while(){ stmt::while_impl::end(this); }

  void constant<unsigned int>::end_while(){ stmt::while_impl::end(this); }

  void constant<long int>::end_while(){ stmt::while_impl::end(this); }

  void constant<unsigned long int>::end_while(){ stmt::while_impl::end(this); }

  void constant<__int64>::end_while(){ stmt::while_impl::end(this); }

  void constant<unsigned __int64>::end_while(){ stmt::while_impl::end(this); }
} // end of namespace c_compiler

void c_compiler::constant<float>::end_while(){ stmt::while_impl::end(this); }
void c_compiler::constant<double>::end_while(){ stmt::while_impl::end(this); }
void c_compiler::constant<long double>::end_while(){ stmt::while_impl::end(this); }
void c_compiler::constant<void*>::end_while(){ stmt::while_impl::end(this); }

void c_compiler::var01::end_while()
{
  using namespace std;
  using namespace stmt;
  using namespace while_impl;
  sweep();
  info* ptr = whiles.back();
  int n = ptr->m_point;
  vector<tac*> stmt;
  copy(code.begin()+n,code.end(),back_inserter(stmt));
  code.resize(n);
  if ( m_one < m_zero ){
    goto3ac* goto1 = new goto3ac;
    stmt.push_back(goto1);
    to3ac* to1 = ptr->m_to;
    goto1->m_to = to1;
    to1->m_goto.push_back(goto1);
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt.begin(),stmt.end());
    vector<tac*>::iterator q = code.begin() + stmt.size() + m_zero - 1;
    delete *q;
    q = code.erase(q);
    sweep(q);
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_zero;
    delete *p;
    p = code.erase(p);
    goto3ac* goto1 = new goto3ac;
    stmt.push_back(goto1);
    to3ac* to1 = ptr->m_to;
    goto1->m_to = to1;
    to1->m_goto.push_back(goto1);
    vector<tac*>::iterator q = code.begin() + m_one - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,stmt.begin(),stmt.end());
  }
  {
    to3ac* to_continue = ptr->m_to;
    const continue_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
    copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
    continue_impl::outer.pop_back();
  }
  {
    to3ac* to_break = static_cast<to3ac*>(code.back());
    const break_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
    copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
    break_impl::outer.pop_back();
  }
  whiles.pop_back();
  delete ptr;
}

namespace c_compiler { namespace stmt {
  namespace do_impl {
    struct info : break_impl::info, continue_impl::info {
      to3ac* m_to;
      info(to3ac* to) : m_to(to) {}
    };
    std::vector<info*> dos;
  }
} } // end of namespace stmt and c_compiler

void c_compiler::stmt::_do()
{
  using namespace do_impl;
  to3ac* to = new to3ac;
  code.push_back(to);
  info* ptr = new info(to);
  dos.push_back(ptr);
  break_impl::outer.push_back(ptr);
  continue_impl::outer.push_back(ptr);
}

void c_compiler::stmt::end_do(to3ac* to2, var* expr)
{
  expr->end_do(to2);
}

void c_compiler::var::end_do(to3ac* to_continue)
{
  using namespace std;
  using namespace stmt;
  using namespace do_impl;
  var* expr = rvalue();
  expr = expr->promotion();
  const type* T = expr->m_type;
  if ( !T->scalar() ){
    using namespace error::stmt::do_while;
    not_scalar(parse::position);
    T = expr->m_type = int_type::create();
  }
  var* zero = integer::create(0);
  zero = zero->cast(T);
  info* ptr = dos.back();
  to3ac* to1 = ptr->m_to;
  goto3ac* goto1 = new goto3ac(goto3ac::NE,expr,zero);
  code.push_back(goto1);
  goto1->m_to = to1;
  to1->m_goto.push_back(goto1);
  {
    const continue_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
    copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
    continue_impl::outer.pop_back();
  }
  to3ac* to_break = new to3ac;
  code.push_back(to_break);
  {
    const break_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
    copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
    break_impl::outer.pop_back();
  }
  dos.pop_back();
  delete ptr;
}

namespace c_compiler { namespace stmt { namespace do_impl {
  template<class T> void end(constant<T>* cons, to3ac* to_continue)
  {
    using namespace std;
    info* ptr = dos.back();
    if ( !cons->zero() ){
      to3ac* to = ptr->m_to;
      goto3ac* go = new goto3ac;
      code.push_back(go);
      go->m_to = to;
      to->m_goto.push_back(go);
    }
    to3ac* to_break = new to3ac;
    code.push_back(to_break);
    {
      const continue_impl::info& p = *ptr;
      for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
      copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
      continue_impl::outer.pop_back();
    }
    {
      const break_impl::info& p = *ptr;
      for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
      copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
      break_impl::outer.pop_back();
    }
    dos.pop_back();
    delete ptr;
  }
} } } // end of namespace while_impl, stmt and c_compiler

namespace c_compiler {
  template<>
  void constant<char>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
  template<>
  void constant<signed char>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
  template<>
  void constant<unsigned char>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
  template<>
  void constant<short int>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
  template<>
  void constant<unsigned short int>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }

  void constant<int>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }

  void constant<unsigned int>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }

  void constant<long int>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }

  void constant<unsigned long int>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }

  void constant<__int64>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }

  void constant<unsigned __int64>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
} // end of namespace c_compiler

void c_compiler::constant<float>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
void c_compiler::constant<double>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
void c_compiler::constant<long double>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }
void c_compiler::constant<void*>::end_do(to3ac* to){ stmt::do_impl::end(this,to); }

void c_compiler::var01::end_do(to3ac* to_continue)
{
  using namespace std;
  using namespace stmt;
  using namespace do_impl;
  sweep();
  info* ptr = dos.back();
  to3ac* to = ptr->m_to;
  vector<tac*>::iterator p = code.begin() + m_one - 1;
  goto3ac* go = static_cast<goto3ac*>(*p);
  go->m_op = opposite[go->m_op];
  go->m_to = to;
  to->m_goto.push_back(go);
  delete *++p;
  p = code.erase(p);
  vector<tac*>::iterator q = code.begin() + m_zero - 1;
  delete *q;
  q = code.erase(q);
  sweep(q);
  {
    const continue_impl::info& v = *ptr;
    for_each(v.begin(),v.end(),bind2nd(ptr_fun(update),to_continue));
    copy(v.begin(),v.end(),back_inserter(to_continue->m_goto));
    continue_impl::outer.pop_back();
  }
  to3ac* to_break = static_cast<to3ac*>(code[m_one]);
  // `static_cast<to3ac*>(*p)' is runtime error in Visual Studio 2017.
  to_break->m_goto.clear();
  {
    const break_impl::info& v = *ptr;
    for_each(v.begin(),v.end(),bind2nd(ptr_fun(update),to_break));
    copy(v.begin(),v.end(),back_inserter(to_break->m_goto));
    break_impl::outer.pop_back();
  }
  dos.pop_back();
  delete ptr;
}

void c_compiler::log01::end_do(to3ac* to_continue)
{
  using namespace std;
  using namespace stmt;
  using namespace do_impl;
  sweep();
  info* ptr = dos.back();
  to3ac* begin = ptr->m_to;
  to3ac* to_break;
  if ( m_one < m_zero ){
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    goto3ac* go = static_cast<goto3ac*>(*p);
    go->m_to = begin;
    begin->m_goto.push_back(go);
    to_break = static_cast<to3ac*>(*++p);
    vector<tac*>::iterator q = code.begin() + m_zero - 1;
    delete *q;
    q = code.erase(q);
    delete *q;
    code.erase(q);
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_goto1;
    goto3ac* goto1 = static_cast<goto3ac*>(*p);
    goto1->m_to = begin;
    begin->m_goto.push_back(goto1);
    p = code.begin() + m_zero - 1;
    goto3ac* goto2 = static_cast<goto3ac*>(*p);
    goto2->m_to = begin;
    begin->m_goto.push_back(goto2);
    delete *++p;  // destroy res := 0
    p = code.erase(p);
    // `p' points to `goto exit'
    delete *p;
    p = code.erase(p);
    // `p' points to `label'
    delete *p;
    p = code.erase(p);
    // `p' points to `res := 1'
    delete *p;
    p = code.erase(p);
    // `p' points to `exit:'
    to_break = static_cast<to3ac*>(*p);
    to_break->m_goto.clear();
  }
  {
    const continue_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
    copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
    continue_impl::outer.pop_back();
  }
  {
    const break_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
    copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
    break_impl::outer.pop_back();
  }
  dos.pop_back();
  delete ptr;
}

namespace c_compiler { namespace stmt {
  namespace for_impl {
    struct info : break_impl::info, continue_impl::info {
      to3ac* m_to;
      int m_point;
      goto3ac* m_goto;
      var* m_expr;
      std::vector<tac*> m_expr3;
      info(to3ac* to, int point) : m_to(to), m_point(point), m_goto(0), m_expr(0) {}
    };
    std::vector<info*> fors;
  }
} } // end of namespace stmt and c_compiler

void c_compiler::stmt::for_expr1(var* expr)
{
  using namespace for_impl;
  if ( expr )
    expr->rvalue();
  to3ac* to = new to3ac;
  code.push_back(to);
  info* ptr = new info(to,code.size());
  fors.push_back(ptr);
  break_impl::outer.push_back(ptr);
  continue_impl::outer.push_back(ptr);
}

bool c_compiler::stmt::for_decl;

void c_compiler::stmt::for_expr2(var* expr)
{
  if ( expr )
    expr->for_expr2();
}

void c_compiler::var::for_expr2()
{
  using namespace std;
  using namespace stmt;
  using namespace for_impl;
  var* expr = rvalue();
  expr = expr->promotion();
  const type* T = expr->m_type;
  if ( !T->scalar() ){
    using namespace error::stmt::_for;
    not_scalar(parse::position);
    T = expr->m_type = int_type::create();
  }
  var* zero = integer::create(0);
  zero = zero->cast(T);
  goto3ac* go = new goto3ac(goto3ac::EQ,expr,zero);
  code.push_back(go);
  info* ptr = fors.back();
  ptr->m_goto = go;
  ptr->m_point = code.size();
  ptr->m_expr = this;
}

namespace c_compiler { namespace stmt { namespace for_impl {
  template<class T> void expr(constant<T>* cons)
  {
    info* ptr = fors.back();
    ptr->m_point = code.size();
    ptr->m_expr = cons;
  }
} } } // end of namespace for_impl, stmt and c_compiler

namespace c_compiler {
  template<>
  void constant<char>::for_expr2(){ stmt::for_impl::expr(this); }
  template<>
  void constant<signed char>::for_expr2(){ stmt::for_impl::expr(this); }
  template<>
  void constant<unsigned char>::for_expr2(){ stmt::for_impl::expr(this); }
  template<>
  void constant<short int>::for_expr2(){ stmt::for_impl::expr(this); }
  template<>
  void constant<unsigned short int>::for_expr2(){ stmt::for_impl::expr(this); }

  void constant<int>::for_expr2(){ stmt::for_impl::expr(this); }

  void constant<unsigned int>::for_expr2(){ stmt::for_impl::expr(this); }

  void constant<long int>::for_expr2(){ stmt::for_impl::expr(this); }

  void constant<unsigned long int>::for_expr2(){ stmt::for_impl::expr(this); }

  void constant<__int64>::for_expr2(){ stmt::for_impl::expr(this); }

  void constant<unsigned __int64>::for_expr2(){ stmt::for_impl::expr(this); }
} // end of namespace c_compiler

void c_compiler::constant<float>::for_expr2(){ stmt::for_impl::expr(this); }
void c_compiler::constant<double>::for_expr2(){ stmt::for_impl::expr(this); }
void c_compiler::constant<long double>::for_expr2(){ stmt::for_impl::expr(this); }
void c_compiler::constant<void*>::for_expr2(){ stmt::for_impl::expr(this); }

void c_compiler::var01::for_expr2()
{
  using namespace stmt;
  using namespace for_impl;
  info* ptr = fors.back();
  ptr->m_point = code.size();
  ptr->m_expr = this;
}

void c_compiler::stmt::for_expr3(var* expr)
{
  using namespace std;
  using namespace for_impl;
  if ( expr )
    expr = expr->rvalue();
  info* ptr = fors.back();
  int n = ptr->m_point;
  copy(code.begin()+n,code.end(),back_inserter(ptr->m_expr3));
  code.resize(n);
}

namespace c_compiler { namespace stmt { namespace for_impl {
  extern void end(int);
} } } // end of namespace for_impl, stmt and c_compiler

void c_compiler::stmt::end_for(int leave)
{
  using namespace for_impl;
  info* ptr = fors.back();
  if ( var* expr2 = ptr->m_expr )
    ptr->m_expr->end_for(leave);
  else
    end(leave);
}

void c_compiler::var::end_for(int leave)
{
  stmt::for_impl::end(leave);
}

void c_compiler::stmt::for_impl::end(int leave)
{
  using namespace std;
  using namespace stmt;
  using namespace for_impl;
  to3ac* to_continue = new to3ac;
  code.push_back(to_continue);
  info* ptr = fors.back();
  vector<tac*>& c = ptr->m_expr3;
  copy(c.begin(),c.end(),back_inserter(code));
  goto3ac* goto2 = new goto3ac;
  code.push_back(goto2);
  to3ac* to2 = ptr->m_to;
  goto2->m_to = to2;
  to2->m_goto.push_back(goto2);
  to3ac* to_break = new to3ac;
  code.push_back(to_break);
  if ( goto3ac* goto3 = ptr->m_goto ){
    to_break->m_goto.push_back(goto3);
    goto3->m_to = to_break;
  }
  {
    const break_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
    copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
    break_impl::outer.pop_back();
  }
  {
    const continue_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
    copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
    continue_impl::outer.pop_back();
  }
  fors.pop_back();
  delete ptr;
  if ( leave )
    c_compiler::parse::block::leave();
}

namespace c_compiler { namespace stmt { namespace for_impl {
  template<class T> void end(constant<T>* cons, int leave)
  {
    using namespace std;
    info* ptr = fors.back();
    assert(ptr->m_expr == cons);
    if ( cons->zero() ){
      int n = ptr->m_point;
      for_each(code.begin()+n,code.end(),[](tac* p){ delete p; });
      code.resize(n);
      vector<tac*>& v = ptr->m_expr3;
      for_each(v.begin(),v.end(),[](tac* p){ delete p; });
    }
    else {
      cons->var::end_for(leave);
      return;
    }
    fors.pop_back();
    delete ptr;
    if ( leave )
      c_compiler::parse::block::leave();
  }
} } } // end of namespace for_impl, stmt and c_compiler

namespace c_compiler {
  template<>
  void constant<char>::end_for(int leave){ stmt::for_impl::end(this,leave); }
  template<>
  void constant<signed char>::end_for(int leave){ stmt::for_impl::end(this,leave); }
  template<>
  void constant<unsigned char>::end_for(int leave){ stmt::for_impl::end(this,leave); }
  template<>
  void constant<short int>::end_for(int leave){ stmt::for_impl::end(this,leave); }
  template<>
  void constant<unsigned short int>::end_for(int leave){ stmt::for_impl::end(this,leave); }

  void constant<int>::end_for(int leave){ stmt::for_impl::end(this,leave); }

  void constant<unsigned int>::end_for(int leave){ stmt::for_impl::end(this,leave); }

  void constant<long int>::end_for(int leave){ stmt::for_impl::end(this,leave); }

  void constant<unsigned long int>::end_for(int leave){ stmt::for_impl::end(this,leave); }

  void constant<__int64>::end_for(int leave){ stmt::for_impl::end(this,leave); }

  void constant<unsigned __int64>::end_for(int leave){ stmt::for_impl::end(this,leave); }
} // end of namespace c_compiler

void c_compiler::constant<float>::end_for(int leave){ stmt::for_impl::end(this,leave); }
void c_compiler::constant<double>::end_for(int leave){ stmt::for_impl::end(this,leave); }
void c_compiler::constant<long double>::end_for(int leave){ stmt::for_impl::end(this,leave); }
void c_compiler::constant<void*>::end_for(int leave){ stmt::for_impl::end(this,leave); }

void c_compiler::var01::end_for(int leave)
{
  using namespace std;
  using namespace stmt;
  using namespace for_impl;
  sweep();
  info* ptr = fors.back();
  assert(ptr->m_expr == this);
  int n = ptr->m_point;
  vector<tac*> stmt;
  copy(code.begin()+n,code.end(),back_inserter(stmt));
  code.resize(n);
  to3ac* to_continue = new to3ac;
  stmt.push_back(to_continue);
  copy(ptr->m_expr3.begin(),ptr->m_expr3.end(),back_inserter(stmt));
  goto3ac* goto1 = new goto3ac;
  stmt.push_back(goto1);
  to3ac* to1 = ptr->m_to;
  goto1->m_to = to1;
  to1->m_goto.push_back(goto1);
  if ( m_one < m_zero ){
    vector<tac*>::iterator p = code.begin() + m_one;
    delete *p;
    p = code.erase(p);
    code.insert(p,stmt.begin(),stmt.end());
    vector<tac*>::iterator q = code.begin() + stmt.size() + m_zero - 1;
    delete *q;
    q = code.erase(q);
    sweep(q);
  }
  else {
    vector<tac*>::iterator p = code.begin() + m_zero;
    delete *p;
    p = code.erase(p);
    vector<tac*>::iterator q = code.begin() + m_one - 1;
    delete *q;
    q = code.erase(q);
    code.insert(q,stmt.begin(),stmt.end());
  }
  {
    const continue_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_continue));
    copy(p.begin(),p.end(),back_inserter(to_continue->m_goto));
    continue_impl::outer.pop_back();
  }
  {
    to3ac* to_break = static_cast<to3ac*>(code.back());
    const break_impl::info& p = *ptr;
    for_each(p.begin(),p.end(),bind2nd(ptr_fun(update),to_break));
    copy(p.begin(),p.end(),back_inserter(to_break->m_goto));
    break_impl::outer.pop_back();
  }
  fors.pop_back();
  delete ptr;
  if ( leave )
    c_compiler::parse::block::leave();
}

void c_compiler::stmt::_goto(usr* u)
{
  using namespace std;
  auto_ptr<usr> sweeper(u);
  string name = u->m_name;
  map<string,label::data_t>::const_iterator p = label::data.find(name);
  if ( p != label::data.end() ){
    using namespace goto_impl;
    const label::data_t& d = p->second;
    if ( goto_impl::outside(scope::current,d.m_scope) ){
      const vector<usr*>& v = label::vm;
      vector<usr*>::const_iterator p = find_if(v.begin(),v.end(),bind2nd(ptr_fun(cmp),d.m_scope));
      if ( p != v.end() ){
        using namespace error::stmt::_goto;
        invalid(parse::position,d.m_file,*p);
      }
    }
    goto3ac* go = new goto3ac;
    code.push_back(go);
    to3ac* to = d.m_to;
    go->m_to = to;
    to->m_goto.push_back(go);
  }
  else {
    goto3ac* go = new goto3ac;
    label::used[name].push_back(label::used_t(go,parse::position,scope::current));
    code.push_back(go);
  }
}

void c_compiler::stmt::_continue()
{
  using namespace continue_impl;
  if ( outer.empty() ){
    using namespace error::stmt::_continue;
    not_within(parse::position);
    return;
  }
  info* info = outer.back();
  goto3ac* go = new goto3ac;
  info->push_back(go);
  code.push_back(go);
}

void c_compiler::stmt::_break()
{
  using namespace break_impl;
  if ( outer.empty() ){
    using namespace error::stmt::_break;
    not_within(parse::position);
    return;
  }
  info* info = outer.back();
  goto3ac* go = new goto3ac;
  info->push_back(go);
  code.push_back(go);
}

void c_compiler::stmt::_return(var* expr)
{
  using namespace std;
  if ( expr )
    expr = expr->rvalue();
  typedef const func_type FUNC;
  FUNC* func = static_cast<FUNC*>(fundef::current->m_usr->m_type);
  const type* T = func->return_type();
  if ( expr ){
    bool discard = false;
    T = c_compiler::expr::assign_impl::valid(T,expr,&discard);
    if ( !T ){
      using namespace error::stmt::_return;
      const type* from = expr->m_type;
      const type* to = func->return_type();
      invalid(parse::position,from,to);
      return;
    }
    expr = expr->cast(T);
  }
  else {
    T = T->unqualified();
    if (T->m_id != type::VOID) {
      using namespace error::stmt::_return;
      invalid(parse::position,void_type::create(),T);
    }
  }
  code.push_back(new return3ac(expr));
}

void c_compiler::stmt::_asm_::action(var* v)
{
  using namespace std;
  assert(scope::current->m_id == scope::BLOCK);
  genaddr* p = v->genaddr_cast();
  var* q = p->m_ref;
  usr* u = static_cast<usr*>(q);
  string name = u->m_name;
  string inst = name[0] == 'L' ? name.substr(2,name.length()-3) : name.substr(1,name.length()-2);
  code.push_back(new asm3ac(inst));
}

void c_compiler::stmt::_asm_::action(var* v, operand_list* p)
{
  // not implemented
  delete p;
  action(v);
}
