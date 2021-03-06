#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"

namespace c_compiler {
  namespace scope_impl {
    using namespace std;
    void usrs_entry(pair<string, vector<usr*> >, int);
    void tags_entry(pair<string, tag*>, int);
    void dump_var(var*, int);
  } // end of namespace scope_impl
} // end of namespace c_compiler

void c_compiler::scope_impl::dump(scope* ptr, int ntab)
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
}

namespace c_compiler { namespace scope_impl {
  void dump_usr(usr*, int);
  void dump_tag(tag*, int);
} } // end of namespace scope_impl and c_compiler

void c_compiler::scope_impl::usrs_entry(std::pair<std::string, std::vector<usr*> > entry, int ntab)
{
  using namespace std;
  const vector<usr*>& v = entry.second;
  for_each(v.begin(),v.end(),bind2nd(ptr_fun(dump_usr),ntab));
}

void
c_compiler::scope_impl::tags_entry(std::pair<std::string, tag*> entry, int ntab)
{
  return dump_tag(entry.second,ntab);
}

namespace c_compiler { namespace scope_impl {
  std::string dump_initial(std::pair<int,var*>);
} } // end of namespace scope_impl and c_compiler

namespace c_compiler { namespace names {
  std::string ref(var*);
} } // end of namespace names and c_compiler

void c_compiler::scope_impl::dump_usr(usr* u, int ntab)
{
  using namespace std;
  for ( int i = 0 ; i < ntab; ++i )
    cout << '\t';
  if (u->m_flag) {
    string s = usr::keyword(u->m_flag);
    if ( !s.empty() )
      cout << s << ' ';
  }
  {
    string name = names::ref(u);
    const type* T = u->m_type;
    T->decl(cout,name);
    if (u->m_flag & usr::WITH_INI) {
      with_initial* p = static_cast<with_initial*>(u);
      cout << '\t';
      const map<int,var*>& v = p->m_value;
      transform(v.begin(),v.end(),ostream_iterator<string>(cout,","),dump_initial);
    }
    else if (u->m_flag & usr::ENUM_MEMBER) {
      enum_member* q = static_cast<enum_member*>(u);
      cout << '\t';
      const usr* v = q->m_value;
      cout << v->m_name;
    }
  }
  cout << '\n';
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

void c_compiler::scope_impl::dump_tag(tag* T, int ntab)
{
  using namespace std;
  for ( int i = 0 ; i < ntab; ++i )
    cout << '\t';
  cout << tag::keyword(T->m_kind) << ' ' << T->m_name << '\n';
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

void c_compiler::scope_impl::dump_var(var* v, int ntab)
{
  using namespace std;
  for ( int i = 0 ; i < ntab; ++i )
    cout << '\t';
  {
    const type* T = v->m_type;
    T->decl(cout,names::ref(v));
    cout << '\n';
  }
}

namespace c_compiler { namespace tac_impl {
  struct table_t : std::map<tac::id_t, void (*)(std::ostream&, const tac*)> {
    table_t();
  } table;
} } // end of namespace tac_impl and c_compiler

void c_compiler::tac_impl::dump(std::ostream& os, const tac* ptr)
{
  table_t::const_iterator p = table.find(ptr->m_id);
  assert(p != table.end());
  (p->second)(os, ptr);
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
  void dump_alloca(std::ostream&, const tac*);
  void dump_va_start(std::ostream&, const tac*);
  void dump_va_arg(std::ostream&, const tac*);
  void dump_va_end(std::ostream&, const tac*);
  void dump_asm(std::ostream&, const tac*);
} } // end of namespace tac_impl and c_compiler

c_compiler::tac_impl::table_t::table_t()
{
  (*this)[tac::ASSIGN] = dump_assign;
  (*this)[tac::MUL] = dump_mul;
  (*this)[tac::DIV] = dump_div;
  (*this)[tac::MOD] = dump_mod;
  (*this)[tac::ADD] = dump_add;
  (*this)[tac::SUB] = dump_sub;
  (*this)[tac::LSH] = dump_lsh;
  (*this)[tac::RSH] = dump_rsh;
  (*this)[tac::AND] = dump_and;
  (*this)[tac::OR] = dump_or;
  (*this)[tac::XOR] = dump_xor;
  (*this)[tac::PARAM] = dump_param;
  (*this)[tac::CALL] = dump_call;
  (*this)[tac::RETURN] = dump_return;
  (*this)[tac::ADDR] = dump_addr;
  (*this)[tac::INVRADDR] = dump_invraddr;
  (*this)[tac::INVLADDR] = dump_invladdr;
  (*this)[tac::UMINUS] = dump_uminus;
  (*this)[tac::TILDE] = dump_tilde;
  (*this)[tac::CAST] = dump_cast;
  (*this)[tac::GOTO] = dump_goto;
  (*this)[tac::TO] = dump_to;
  (*this)[tac::LOFF] = dump_loff;
  (*this)[tac::ROFF] = dump_roff;
  (*this)[tac::ALLOCA] = dump_alloca;
  (*this)[tac::VASTART] = dump_va_start;
  (*this)[tac::VAARG] = dump_va_arg;
  (*this)[tac::VAEND] = dump_va_end;
  (*this)[tac::ASM] = dump_asm;
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

void c_compiler::tac_impl::dump_alloca(std::ostream& os, const tac* ptr)
{
  using namespace std;
  string x = names::ref(ptr->x);
  string y = names::ref(ptr->y);
  os << "alloca " << x << ", " << y;
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

namespace c_compiler {
  using namespace std;
  using namespace optimize::basic_block;
  namespace names {
    table<info_t*> bbs;
    string refb(info_t*);
  } // end of namespace names
  namespace live_var {
    void liveB(const pair<info_t*, set<var*> >& p)
    {
      info_t* B = p.first;
      cout << '\t' << names::refb(B) << " :";
      const set<var*>& s = p.second;
      for_each(s.begin(),s.end(),[](var* v){ cout << ' ' << names::ref(v); });
      cout << '\n';
    }
    void dump(string name, const map<info_t*, set<var*> >& m)
    {
      cout << name << '\n';
      for_each(m.begin(),m.end(),liveB);
    }
  } // end of namespace live_var
} // end of namespace c_compiler

std::string c_compiler::names::refb(optimize::basic_block::info_t* B)
{
  using namespace std;
  if ( cmdline::simple_medium ){
    table<optimize::basic_block::info_t*>::const_iterator p = bbs.find(B);
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
