#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"

namespace c_compiler { namespace scope_impl {
  int usrs_entry(std::pair<std::string, std::vector<usr*> >, int);
  int tags_entry(std::pair<std::string, tag*>, int);
  int dump_var(var*, int);
} } // end of namespace scope_impl and c_compiler

int c_compiler::scope_impl::dump(scope* ptr, int ntab)
{
  using namespace std;
  const map<string, vector<usr*> >& u = ptr->m_usrs;
  for_each(u.begin(),u.end(),bind2nd(ptr_fun(usrs_entry),ntab));
  const map<string, tag*>& t = ptr->m_tags;
  for_each(t.begin(),t.end(),bind2nd(ptr_fun(tags_entry),ntab));
  if ( ptr->m_id == scope::BLOCK ){
    block* b = static_cast<block*>(ptr);
    const vector<var*>& v = b->m_vars;
    for_each(v.begin(),v.end(),bind2nd(ptr_fun(dump_var),ntab));
  }
  const vector<scope*>& c = ptr->m_children;
  for_each(c.begin(),c.end(),bind2nd(ptr_fun(scope_impl::dump),ntab+1));
  return 0;
}

namespace c_compiler { namespace scope_impl {
  int dump_usr(usr*, int);
  int dump_tag(tag*, int);
} } // end of namespace scope_impl and c_compiler

int c_compiler::scope_impl::usrs_entry(std::pair<std::string, std::vector<usr*> > entry, int ntab)
{
  using namespace std;
  const vector<usr*>& v = entry.second;
  for_each(v.begin(),v.end(),bind2nd(ptr_fun(dump_usr),ntab));
  return 0;
}

int c_compiler::scope_impl::tags_entry(std::pair<std::string, tag*> entry, int ntab)
{
  return dump_tag(entry.second,ntab);
}

namespace c_compiler { namespace scope_impl {
  std::string dump_initial(std::pair<int,var*>);
} } // end of namespace scope_impl and c_compiler

namespace c_compiler { namespace names {
  std::string ref(var*);
} } // end of namespace names and c_compiler

int c_compiler::scope_impl::dump_usr(usr* u, int ntab)
{
  using namespace std;
  for ( int i = 0 ; i < ntab; ++i )
    cout << '\t';
  if ( u->m_flag ){
    string s = usr::keyword(u->m_flag);
    if ( !s.empty() )
      cout << s << ' ';
  }
  {
    string name = names::ref(u);
    const type* T = u->m_type;
    T->decl(cout,name);
    if ( u->m_flag & usr::WITH_INI ){
	  with_initial* p = static_cast<with_initial*>(u);
      cout << '\t';
      const map<int,var*>& v = p->m_value;
      transform(v.begin(),v.end(),ostream_iterator<string>(cout,","),dump_initial);
    }
    else {
      if ( enum_member* q = dynamic_cast<enum_member*>(u) ){
        cout << '\t';
        const usr* v = q->m_value;
        cout << v->m_name;
      }
    }
  }
  cout << '\n';
  return 0;
}

std::string c_compiler::scope_impl::dump_initial(std::pair<int,var*> p)
{
  using namespace std;
  ostringstream os;
  os << '(' << p.first << ',';
  var* v = p.second;
  addrof* addr = dynamic_cast<addrof*>(v);
  if ( addr ){
    os << "addrof(";
    v = addr->m_ref;
  }
  os << names::ref(v);
  if ( addr ){
    os << ')';
    if ( int offset = addr->m_offset )
      os << '+' << offset;
  }
  os << ')';
  return os.str();
}

int c_compiler::scope_impl::dump_tag(tag* T, int ntab)
{
  using namespace std;
  for ( int i = 0 ; i < ntab; ++i )
    cout << '\t';
  cout << tag::keyword(T->m_kind) << ' ' << T->m_name << '\n';
  return 0;
}

namespace c_compiler { namespace names {
  template<class C> struct table : std::map<C, std::string> {
    int m_cnt;
    void reset()
    {
      std::map<C, std::string>::clear();
      m_cnt = 0;
    }
  };
  table<var*> vars;
} } // end of namespace names and c_compiler

std::string c_compiler::names::ref(var* v)
{
  using namespace std;
  if ( usr* u = v->usr_cast() ){
    string name = u->m_name;
    if ( name[0] != '.' )
      return name;
    if ( constant<int>* p = dynamic_cast<constant<int>*>(u) ){
      int n = p->m_value;
      ostringstream os;
      os << n;
      return os.str();
    }
    if ( constant<char>* p = dynamic_cast<constant<char>*>(u) ){
      char c = p->m_value;
      if ( c > 0 && (isgraph(c) || c == ' ' || c == '\t' )){
        ostringstream os;
        os << "'" << c << "'";
        return os.str();
      }
    }
    return name;
  }
  if ( cmdline::simple_medium ){
    map<var*, string>::const_iterator p = vars.find(v);
    if ( p != vars.end() )
      return p->second;
    ostringstream os;
    os << 't' << vars.m_cnt++;
    return vars[v] = os.str();
  }
  ostringstream os;
  os << ".var" << v;
  return os.str();
}

int c_compiler::scope_impl::dump_var(var* v, int ntab)
{
  using namespace std;
  for ( int i = 0 ; i < ntab; ++i )
    cout << '\t';
  {
    const type* T = v->m_type;
    T->decl(cout,names::ref(v));
    cout << '\n';
  }
  return 0;
}

namespace c_compiler { namespace tac_impl {
  struct table : std::map<std::string, void (*)(std::ostream&, const tac*)> {
    table();
  } m_table;
} } // end of namespace tac_impl and c_compiler

void c_compiler::tac_impl::dump(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string name = typeid(*ptr).name();
  m_table[name](os,ptr);
}

namespace c_compiler { namespace tac_impl {
  void dump_assign(std::ostream&, const tac*);
  void dump_mul(std::ostream&, const tac*);
  void dump_div(std::ostream&, const tac*);
  void dump_mod(std::ostream&, const tac*);
  void dump_add(std::ostream&, const tac*);
  void dump_sub(std::ostream&, const tac*);
  void dump_lsh(std::ostream&, const tac*);
  void dump_rsh(std::ostream&, const tac*);
  void dump_and(std::ostream&, const tac*);
  void dump_or(std::ostream&, const tac*);
  void dump_xor(std::ostream&, const tac*);
  void dump_param(std::ostream&, const tac*);
  void dump_call(std::ostream&, const tac*);
  void dump_return(std::ostream&, const tac*);
  void dump_addr(std::ostream&, const tac*);
  void dump_invraddr(std::ostream&, const tac*);
  void dump_invladdr(std::ostream&, const tac*);
  void dump_uminus(std::ostream&, const tac*);
  void dump_tilde(std::ostream&, const tac*);
  void dump_cast(std::ostream&, const tac*);
  void dump_goto(std::ostream&, const tac*);
  void dump_to(std::ostream&, const tac*);
  void dump_loff(std::ostream&, const tac*);
  void dump_roff(std::ostream&, const tac*);
  void dump_alloc(std::ostream&, const tac*);
  void dump_dealloc(std::ostream&, const tac*);
  void dump_va_start(std::ostream&, const tac*);
  void dump_va_arg(std::ostream&, const tac*);
  void dump_va_end(std::ostream&, const tac*);
  void dump_asm(std::ostream&, const tac*);
} } // end of namespace tac_impl and c_compiler

c_compiler::tac_impl::table::table()
{
  (*this)[typeid(assign3ac).name()] = dump_assign;
  (*this)[typeid(mul3ac).name()] = dump_mul;
  (*this)[typeid(div3ac).name()] = dump_div;
  (*this)[typeid(mod3ac).name()] = dump_mod;
  (*this)[typeid(add3ac).name()] = dump_add;
  (*this)[typeid(sub3ac).name()] = dump_sub;
  (*this)[typeid(lsh3ac).name()] = dump_lsh;
  (*this)[typeid(rsh3ac).name()] = dump_rsh;
  (*this)[typeid(and3ac).name()] = dump_and;
  (*this)[typeid(or3ac).name()] = dump_or;
  (*this)[typeid(xor3ac).name()] = dump_xor;
  (*this)[typeid(param3ac).name()] = dump_param;
  (*this)[typeid(call3ac).name()] = dump_call;
  (*this)[typeid(return3ac).name()] = dump_return;
  (*this)[typeid(addr3ac).name()] = dump_addr;
  (*this)[typeid(invraddr3ac).name()] = dump_invraddr;
  (*this)[typeid(invladdr3ac).name()] = dump_invladdr;
  (*this)[typeid(uminus3ac).name()] = dump_uminus;
  (*this)[typeid(tilde3ac).name()] = dump_tilde;
  (*this)[typeid(cast3ac).name()] = dump_cast;
  (*this)[typeid(goto3ac).name()] = dump_goto;
  (*this)[typeid(to3ac).name()] = dump_to;
  (*this)[typeid(loff3ac).name()] = dump_loff;
  (*this)[typeid(roff3ac).name()] = dump_roff;
  (*this)[typeid(alloc3ac).name()] = dump_alloc;
  (*this)[typeid(dealloc3ac).name()] = dump_dealloc;
  (*this)[typeid(va_start3ac).name()] = dump_va_start;
  (*this)[typeid(va_arg3ac).name()] = dump_va_arg;
  (*this)[typeid(va_end3ac).name()] = dump_va_end;
  (*this)[typeid(asm3ac).name()] = dump_asm;
}

void c_compiler::tac_impl::dump_assign(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := " << y;
}

namespace c_compiler { namespace tac_impl {
  void dump_bin(std::ostream&, const tac*, std::string);
} } // end of namespace tac_impl and c_compiler

void c_compiler::tac_impl::dump_mul(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"*");
}

void c_compiler::tac_impl::dump_div(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"/");
}

void c_compiler::tac_impl::dump_mod(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"%");
}

void c_compiler::tac_impl::dump_add(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"+");
}

void c_compiler::tac_impl::dump_sub(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"-");
}

void c_compiler::tac_impl::dump_lsh(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"<<");
}

void c_compiler::tac_impl::dump_rsh(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,">>");
}

void c_compiler::tac_impl::dump_and(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"&");
}

void c_compiler::tac_impl::dump_or(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"|");
}

void c_compiler::tac_impl::dump_xor(std::ostream& os, const tac* ptr)
{
  dump_bin(os,ptr,"^");
}

void c_compiler::tac_impl::dump_bin(std::ostream& os, const tac* ptr, std::string op)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << " := " << y << ' ' << op << ' ' << z;
}

void c_compiler::tac_impl::dump_param(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  os << "param " << y;
}

void c_compiler::tac_impl::dump_call(std::ostream& os, const tac* ptr)
{
  using namespace std;
  if ( ptr->x ){
    string x = names::ref(ptr->x);
    os << x << " := ";
  }
  string y = names::ref(ptr->y);
  os << "call " << y;
}

void c_compiler::tac_impl::dump_return(std::ostream& os, const tac* ptr)
{
  using namespace std;
  os << "return";
  if ( ptr->y ){
    string y = names::ref(ptr->y);
    os << ' ' << y;
  }
}

namespace c_compiler { namespace tac_impl {
  void dump_una(std::ostream&, const tac*, std::string);
} } // end of namespace tac_impl and c_compiler

void c_compiler::tac_impl::dump_addr(std::ostream& os, const tac* ptr)
{
  dump_una(os,ptr,"&");
}

void c_compiler::tac_impl::dump_invraddr(std::ostream& os, const tac* ptr)
{
  dump_una(os,ptr,"*");
}

void c_compiler::tac_impl::dump_invladdr(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << '*' << y << " := " << z;
}

void c_compiler::tac_impl::dump_uminus(std::ostream& os, const tac* ptr)
{
  dump_una(os,ptr,"-");
}

void c_compiler::tac_impl::dump_tilde(std::ostream& os, const tac* ptr)
{
  dump_una(os,ptr,"~");
}

void c_compiler::tac_impl::dump_cast(std::ostream& os, const tac* ptr)
{
  using namespace std;
  ostringstream tmp;
  tmp << '(';
  const cast3ac* tc = static_cast<const cast3ac*>(ptr);
  const type* type = tc->m_type;
  type->decl(tmp,"");
  tmp << ')';
  dump_una(os,ptr,tmp.str());
}

void c_compiler::tac_impl::dump_una(std::ostream& os, const tac* ptr, std::string op)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := " << op << y;
}

namespace c_compiler { namespace tac_impl {
  std::string op2str(goto3ac::op op)
  {
    switch ( op ){
    case goto3ac::EQ: return "==";
    case goto3ac::NE: return "!=";
    case goto3ac::LE: return "<=";
    case goto3ac::GE: return ">=";
    case goto3ac::LT: return "<";
    case goto3ac::GT: return ">";
    default:          return "";
    }
  }
} } // end of namespace tac_impl and c_compiler

namespace c_compiler { namespace names {
  std::string ref(to3ac*);
} } // end of namespace names and c_compiler

void c_compiler::tac_impl::dump_goto(std::ostream& os, const tac* ptr)
{
  using namespace std;
  const goto3ac* p = static_cast<const goto3ac*>(ptr);
  string op = op2str(p->m_op);
  if ( op.empty() )
    os << "goto " << names::ref(p->m_to);
  else {
    string y = names::ref(ptr->y);
    string z = names::ref(ptr->z);
    os << "if " << y << ' ' << op << ' ' << z << " goto " << names::ref(p->m_to);
  }
}

namespace c_compiler { namespace names {
  table<to3ac*> labels;
} } // end of namespace names and c_compiler

std::string c_compiler::names::ref(to3ac* label)
{
  using namespace std;
  if ( cmdline::simple_medium ){
    map<to3ac*, string>::const_iterator p = labels.find(label);
    if ( p != labels.end() )
      return p->second;
    ostringstream os;
    os << "label" << labels.m_cnt++;
    return labels[label] = os.str();
  }
  else {
    ostringstream os;
    os << "label" << label;
    return os.str();
  }
}

void c_compiler::tac_impl::dump_to(std::ostream& os, const tac* ptr)
{
  using namespace std;
  tac* x = const_cast<tac*>(ptr);
  to3ac* y = static_cast<to3ac*>(x);
  string label = names::ref(y);
  os << label << ':';
}

void c_compiler::tac_impl::dump_loff(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << '[' << y << ']' << " := " << z;
}

void c_compiler::tac_impl::dump_roff(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << x << " := " << y << '[' << z << ']';
}

void c_compiler::tac_impl::dump_alloc(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << "alloc " << x << ", " << y;
}

void c_compiler::tac_impl::dump_dealloc(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string y = names::ref(ptr->y);
  string z = names::ref(ptr->z);
  os << "dealloc " << y << ", " << z;
}

void c_compiler::tac_impl::dump_va_start(std::ostream& os, const tac* ptr)
{
	using namespace std;
	string x = names::ref(ptr->x);
	string y = names::ref(ptr->y);
	os << x << " := va_start " << y;
}

void c_compiler::tac_impl::dump_va_arg(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << x << " := va_arg " << y << " ,";
  const va_arg3ac* va = static_cast<const va_arg3ac*>(ptr);
  const type* type = va->m_type;
  type->decl(os,"");
}

void c_compiler::tac_impl::dump_va_end(std::ostream& os, const tac* ptr)
{
	using namespace std;
	string y = names::ref(ptr->y);
	os << "va_end " << y;
}

void c_compiler::tac_impl::dump_asm(std::ostream& os, const tac* ptr)
{
  using namespace std;
  const asm3ac* p = static_cast<const asm3ac*>(ptr);
  os << "asm " << '"' << p->m_inst << '"' ;
}

namespace c_compiler { namespace live_var {
  int live1(const std::pair<optimize::basic_block::info*, std::set<var*> >&);
} } // end of namespace live_var and cxx_compiler

void
c_compiler::live_var::dump(std::string name,
                           const std::map<optimize::basic_block::info*, std::set<var*> >& m)
{
  using namespace std;
  cout << name << '\n';
  for_each(m.begin(),m.end(),live1);
}

namespace c_compiler { namespace live_var {
  std::string refb(optimize::basic_block::info*);
} } // end of namespace live_var and cxx_compiler

namespace c_compiler { namespace live_var {
  int live2(var*);
} } // end of namespace live_var and cxx_compiler

namespace c_compiler { namespace names {
  std::string refb(optimize::basic_block::info*);
} } // end of namespace names, dump and cxx_compiler

int
c_compiler::live_var::live1(const std::pair<optimize::basic_block::info*, std::set<var*> >& p)
{
  using namespace std;
  optimize::basic_block::info* B = p.first;
  cout << '\t' << names::refb(B) << " :";
  const set<var*>& v = p.second;
  for_each(v.begin(),v.end(),live2);
  cout << '\n';
  return 0;
}

int c_compiler::live_var::live2(var* v)
{
  using namespace std;
  cout << ' ' << names::ref(v);
  return 0;
}

namespace c_compiler { namespace names {
  table<optimize::basic_block::info*> bbs;
} } // end of namespace names, dump and cxx_compiler

std::string c_compiler::names::refb(optimize::basic_block::info* B)
{
  using namespace std;
  if ( cmdline::simple_medium ){
    table<optimize::basic_block::info*>::const_iterator p = bbs.find(B);
    if ( p != bbs.end() )
      return p->second;
    ostringstream os;
    os << 'B' << bbs.m_cnt++;
    return bbs[B] = os.str();
  }
  ostringstream os;
  os << 'B' << B;
  return os.str();
}

void c_compiler::names::reset()
{
  vars.reset();
  labels.reset();
  bbs.reset();
}
