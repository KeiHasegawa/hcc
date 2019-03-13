#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"
#include "c_y.h"

namespace c_compiler { namespace initializer {
  usr* argument::dst;
  int assign(var*, argument&);
  int lst(parse::initializer_list*, argument&);
} } // end of namespace initializer and c_compiler

int c_compiler::initializer::eval(parse::initializer* ini, argument& arg)
{
  return ini->first ? assign(ini->first,arg) :
    initializer::lst(ini->second,arg);
}

namespace c_compiler { namespace initializer {
  int char_array_string(var*, argument&);
  struct merge {
    std::map<int,var*>& v;
    int offset;
    merge(std::map<int,var*>& vv, int o) : v(vv), offset(o) {}
    void operator()(const std::pair<int,var*>& p)
    {
      v[p.first+offset] = p.second;
    }
  };
  int bit_field(var*, argument&);
} } // end of namespace initializer and c_compiler

namespace c_compiler { namespace initializer {
  int assign_special(var*, argument&);
} } // end of namespace initializer and c_compiler

int c_compiler::initializer::assign(var* y, argument& arg)
{
  using namespace std;
  using namespace decl;
  y = y->rvalue();
  if ( static_storage_duration(argument::dst) && !y->isconstant(true) ){
    using namespace error::decl::initializer;
    not_constant(parse::position);
    return arg.off;
  }
  if ( int r = char_array_string(y,arg) )
    return r;
  const type* T = arg.T;
  if ( arg.nth >= 0 ){
    pair<int, const type*> ret = T->current(arg.nth);
    if ( ret.first < 0 ){
      using namespace error::decl::initializer;
      exceed(argument::dst);
      return arg.off;
    }
    T = ret.second;
    if ( !T->scalar() && y->m_type->scalar() )
      return assign_special(y,arg);
    arg.off_max = max(arg.off_max, arg.off = ret.first);
  }
  bool discard = false;
  T = expr::assign_impl::valid(T,y,&discard);
  if ( !T ){
    using namespace error::decl::initializer;
    invalid_assign(parse::position,argument::dst,discard);
    return arg.off;
  }
  typedef const bit_field_type BF;
  if ( T->m_id == type::BIT_FIELD )
    return bit_field(y,arg);
  y = y->cast(T);
  if ( y->addrof_cast() ){
    vector<var*>& v = garbage;
    vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),y);
    assert(p != v.rend());
    v.erase(p.base()-1);
  }
  arg.V[arg.off] = y;
  arg.off_max = max(arg.off_max, arg.off += T->size());
  if ( arg.nth >= 0 )
    arg.nth_max = max(arg.nth_max,++arg.nth);
  return arg.off;
}

namespace c_compiler { namespace initializer { namespace char_array_string_impl {
  usr* is_string(var*);
  const array_type* char_array(const type*, int, bool*);
  struct eval {
    argument& arg;
    bool m_wide;
    bool m_escape;
    int m_hex_mode;
    char m_prev;
    bool m_shiftjis_state;
    int m_jis_state;
    int m_euc_state;
    eval(argument& a, bool wide)
      : arg(a), m_wide(wide), m_escape(false), m_hex_mode(0), m_prev(0),
        m_shiftjis_state(false), m_jis_state(0), m_euc_state(0) {}
    int operator()(int);
  };
} } } // end of namespace char_array_string_impl, initializer and c_compiler

int c_compiler::initializer::char_array_string(var* y, argument& arg)
{
  using namespace std;
  usr* s = char_array_string_impl::is_string(y);
  if ( !s )
    return 0;
  typedef const array_type ARRAY;
  bool brace = false;
  ARRAY* array = char_array_string_impl::char_array(arg.T,arg.nth,&brace);
  if ( !array )
    return 0;
  const type* T = array->element_type();
  var* zero = integer::create(char(0));
  zero = zero->cast(T);
  string name = s->m_name;
  bool wide = name[0] == 'L';
  name = name.substr(1+wide,name.length()-(2+wide));
  int dim = array->dim();
  map<int,var*> u;
  argument tmp(array,u,0,0,0,0,0,0);
  for_each(name.begin(),name.end(),char_array_string_impl::eval(tmp,wide));
  if ( tmp.nth != dim || !dim ){
    u[tmp.off] = zero;
    tmp.off += wide ? generator::wchar::type->size() : 1;
    tmp.off_max = max(tmp.off_max,tmp.off);
    tmp.nth_max = max(tmp.nth_max,++tmp.nth);
  }
  fill_zero(tmp);
  for_each(u.begin(),u.end(),merge(arg.V,arg.off));
  arg.nth_max = max(arg.nth_max,brace ? arg.nth += tmp.nth_max : ++arg.nth);
  arg.off_max = max(arg.off_max, arg.off += tmp.off);
  return arg.off;
}

c_compiler::usr*
c_compiler::initializer::char_array_string_impl::is_string(var* y)
{
  if ( genaddr* ptr = y->genaddr_cast() ){
    if ( usr* u = ptr->m_ref->usr_cast() ){
      std::string name = u->m_name;
      if ( name[0] == '"' )
        return u;
      if ( name[0] == 'L' && name.length() > 1 && name[1] == '"' )
        return u;
    }
  }
  return 0;
}

namespace c_compiler { namespace initializer {
  namespace char_array_string_impl {
    const array_type* char_array(const type*);
  }
} } // end of namespace initializer and c_compiler

const c_compiler::array_type* c_compiler::initializer::char_array_string_impl::char_array(const type* T)
{
  if ( T->m_id != type::ARRAY )
    return 0;
  typedef const array_type ARRAY;
  ARRAY* array = static_cast<ARRAY*>(T);
  T = array->element_type();
  T = T->unqualified();
  if ( T != char_type::create() && T != generator::wchar::type )
    return 0;
  return array;
}

const c_compiler::array_type*
c_compiler::initializer::char_array_string_impl::char_array(const type* T, int nth, bool* brace)
{
  using namespace std;
  if ( nth < 0 )
    return char_array(T);
  if ( nth == 0 ){
    if ( const array_type* r = char_array(T) ){
      *brace = true;
      return r;
    }
  }
  pair<int, const type*> ret = T->current(nth);
  if ( ret.first < 0 )
    return 0;
  return char_array(ret.second);
}

namespace c_compiler {
  namespace character_impl {
    usr* common(int v)
    {
      switch (generator::wchar::id) {
      case type::SHORT:
        return integer::create((short)v);
      case type::USHORT:
        return integer::create((unsigned short)v);
      case type::INT:
        return integer::create((int)v);
      case type::UINT:
        return integer::create((unsigned)v);
      case type::LONG:
        return integer::create((long)v);
      default:
        assert(generator::wchar::id == type::ULONG);
        return integer::create((unsigned long)v);
      }
    }
    usr* integer_hex(bool wide, int a, int b)
    {
      int v = a << 4 | b;
      return wide ? common(v) : integer::create(char(v));
    }
    usr* integer_create(bool wide, char prev, int c)
    {
      int v = prev << 8 | c;
      return wide ? common(v) : integer::create(char(v));
    }
    usr* integer_create(bool wide, int c)
    {
      return wide ? common(c) : integer::create(char(c));
    }
  } // end of namespace character_impl
} // end of namespace c_compiler

int c_compiler::initializer::char_array_string_impl::eval::operator()(int c)
{
  using namespace std;
  if ( c == '\\' && !m_escape ){
    m_escape = true;
    return arg.off;
  }
  if ( m_escape && c == 'x' ){
    m_hex_mode = 1;
    m_escape = false;
    return arg.off;
  }
  if ( m_hex_mode ){
    if ( isxdigit(c) ){
      if ( ++m_hex_mode < 3 ){
        m_prev = c;
        return arg.off;
      }
    }
  }
  c = (unsigned char)c;
  if ( !m_shiftjis_state && ( 129 <= c && c <= 159 || 224 <= c && c <= 239 ) ){
    m_shiftjis_state = true;
    m_prev = c;
    return arg.off;
  }
  if ( m_jis_state == 0 && c == 0x1b ){
    m_jis_state = 1;
    return arg.off;
  }
  if ( m_jis_state == 1 && c == 0x24 ){
    m_jis_state = 2;
    return arg.off;
  }
  if ( m_jis_state == 2 && c == 0x42 ){
    m_jis_state = 3;
    return arg.off;
  }
  if ( m_jis_state == 3 ){
    if ( c == 0x1b ){
      m_jis_state = 5;
      return arg.off;
    }
    m_prev = c;
    m_jis_state = 4;
    return arg.off;
  }
  if ( m_jis_state == 4 )
    m_jis_state = 3;
  if ( m_jis_state == 5 && c == 0x28 ){
    m_jis_state = 6;
    return arg.off;
  }
  if ( m_jis_state == 6 && c == 0x42 ){
    m_jis_state = 0;
    return arg.off;
  }
  if ( m_euc_state == 0 && c == 0x8e ){
    m_euc_state = 1;
    return arg.off;
  }
  if ( !m_shiftjis_state && m_euc_state == 0 && 0xa1 <= c && c <= 0xfe ){
    m_prev = c;
    m_euc_state = 2;
    return arg.off;
  }

  usr* y;
  if ( m_escape ){
    ostringstream os;
    if ( m_wide )
      os << 'L';
    os << "'";
    os << '\\' << char(c) << "'";
    y = character::create(os.str());
    m_escape = false;
  }
  else if ( m_hex_mode ){
    int a = isdigit(m_prev) ? m_prev - '0' : m_prev - 'a' + 10;
    int b = isdigit(c) ? c - '0' : c - 'a' + 10;
    y = character_impl::integer_hex(m_wide, a, b);
    m_hex_mode = false;
  }
  else if ( m_shiftjis_state ){
    assert(64 <= c && c <= 126 || 128 <= c && c <= 252);
    y = character_impl::integer_create(m_wide, m_prev, c);
    m_shiftjis_state = false;
  }
  else if ( m_jis_state == 3 )
    y = character_impl::integer_create(m_wide, m_prev, c);
  else if ( m_euc_state == 1 ){
    y = integer::create(char(c));
    m_euc_state = 0;
  }
  else if ( m_euc_state == 2 ){
    y = character_impl::integer_create(m_wide, m_prev, c);
    m_euc_state = 0;
  }
  else
    y = character_impl::integer_create(m_wide, c);
  arg.V[arg.off] = y;
  arg.nth_max = max(arg.nth_max,++arg.nth);
  arg.off += m_wide ? generator::wchar::type->size(): 1;
  arg.off_max = max(arg.off_max, arg.off);
  return arg.off;
}

namespace c_compiler { namespace initializer {
  int order(const record_type*, int);
  int member_size(const record_type*);
} } // end of namespace initializer and c_compiler

int c_compiler::initializer::assign_special(var* y, argument& arg)
{
  using namespace std;
  const type* T = arg.T;
  pair<int, const type*> ret = T->current(arg.nth);
  T = ret.second;
  int size = T->size();
  if ( !size ){
    using namespace error::decl::initializer;
    not_object(argument::dst);
    return 0;
  }
  typedef const array_type ARRAY;
  ARRAY* array = T->m_id == type::ARRAY ? static_cast<ARRAY*>(T) : 0;
  typedef const record_type REC;
  REC* rec = array ? 0 : static_cast<REC*>(T->unqualified());
  map<int,var*> u;
  int offset = arg.off % size;
  int nth = array ? offset / array->element_type()->size() : order(rec,offset);
  argument tmp(T,u,offset,offset,nth,nth,arg.list_pos,arg.list_len);
  assign(y,tmp);
  fill_zero(tmp);
  for_each(u.begin(),u.end(),merge(arg.V,ret.first));
  if ( tmp.nth == (array ? array->dim() : member_size(rec)) )
    arg.nth_max = max(arg.nth_max,++arg.nth), tmp.off = 0;
  arg.off_max = max(arg.off_max, arg.off = arg.nth * size + tmp.off);
  return arg.off;
}

int c_compiler::initializer::order(const record_type* rec, int y)
{
  using namespace std;
  tag* T = rec->get_tag();
  if ( T->m_kind == tag::UNION )
    return 0;

  int nth = 0;
  while ( 1 ){
    pair<int, const type*> ret = rec->current(nth);
    int x = ret.first;
    const type* T = ret.second;
    if ( y < x + T->size() || y <= x )
      break;
    ++nth;
  }
  return nth;
}

int c_compiler::initializer::member_size(const record_type* rec)
{
  using namespace std;
  tag* T = rec->get_tag();
  if ( T->m_kind == tag::UNION )
    return 1;
  return rec->member().size();
}

int c_compiler::initializer::lst(parse::initializer_list* y, argument& arg)
{
  using namespace std;
  int off = arg.off;
  const type* T = arg.T;
  if ( arg.nth >= 0 ){
    pair<int, const type*> ret = T->current(arg.nth);
    if ( ret.first < 0 ){
      using namespace error::decl::initializer;
      exceed(argument::dst);
      return arg.off;
    }
    off = ret.first;
    T = ret.second;
  }
  map<int,var*> u;
  argument tmp(T,u,0,0,0,0,0,y->size());
  for_each(y->begin(),y->end(),initializer_list::eval(tmp));
  fill_zero(tmp);
  for_each(u.begin(),u.end(),merge(arg.V,off));
  if ( arg.nth >= 0 )
    arg.nth_max = max(arg.nth_max, ++arg.nth);
  arg.off_max = max(arg.off_max, arg.off = off + tmp.off_max);
  return arg.off;
}

namespace c_compiler { namespace initializer_list {
  int designation_handler(parse::designation*, parse::initializer*, initializer::argument&);
} } // end of namespace initializer_list and c_compiler

int c_compiler::initializer_list::eval::operator()(parse::ppair<parse::designation*, parse::initializer*>* p)
{
  using namespace initializer;
  if ( parse::designation* d = p->first )
    return designation_handler(d,p->second,arg);
  else
    return ++arg.list_pos, initializer::eval(p->second,arg);
}

namespace c_compiler { namespace initializer_list {
  int designator(parse::designator*, initializer::argument*);
  const type* complete(const type*, int);
} } // end of namespace initializer_list and c_compiler

int
c_compiler::initializer_list::designation_handler(parse::designation* des,
                                                  parse::initializer* ini,
                                                  initializer::argument& arg)
{
  using namespace std;
  using namespace initializer;
  
  map<int, var*> u1;
  argument tmp1(arg.T,u1,0,0,-1,-1,-1,-1);
  for_each(des->begin(),des->end(),bind2nd(ptr_fun(designator),&tmp1));
  u1.erase(u1.begin(),u1.lower_bound(arg.off_max));
  for_each(u1.begin(),u1.end(),initializer::merge(arg.V,0));
  arg.nth_max = max(arg.nth_max, arg.nth = tmp1.nth + 1);

  map<int, var*> u2;
  ++arg.list_pos;
  argument tmp2(tmp1.T,u2,0,0,-1,-1,arg.list_pos,arg.list_len);
  initializer::eval(ini,tmp2);
  for_each(u2.begin(),u2.end(),initializer::merge(arg.V,tmp1.off));
  arg.off_max = max(arg.off_max, arg.off = tmp1.off + tmp2.off);
  
  map<int, var*> u3;
  argument tmp3(complete(arg.T,arg.off_max),u3,0,0,0,0,arg.list_pos,arg.list_len);
  fill_zero(tmp3);
  u3.erase(u3.begin(),u3.lower_bound(arg.off_max));
  for_each(u3.begin(),u3.end(),initializer::merge(arg.V,0));
  if ( tmp3.off_max )
    arg.off_max = max(arg.off_max, arg.off = tmp3.off_max);
  
  return arg.off;
}

namespace c_compiler { namespace initializer_list {
  namespace designator_impl {
    int subscripting(var*, initializer::argument&);
    int dot(usr*, initializer::argument&);
  }
} } // end of namespace initializer_list and c_compiler

int c_compiler::initializer_list::designator(parse::designator* d, initializer::argument* arg)
{
  using namespace std;
  var* cexpr = d->first;
  return cexpr ? designator_impl::subscripting(cexpr,*arg)
    : designator_impl::dot(d->second,*arg);
}

int c_compiler::initializer_list::designator_impl::subscripting(var* cexpr, initializer::argument& arg)
{
  using namespace std;
  using namespace initializer;
  if ( arg.T->m_id != type::ARRAY ){
    using namespace error::decl::initializer::designator;
    invalid_subscripting(parse::position,arg.T);
    return arg.off;
  }
  typedef const array_type ARRAY;
  ARRAY* array = static_cast<ARRAY*>(arg.T);
  if ( !cexpr->m_type->integer() ){
    using namespace error::decl::initializer::designator;
    not_integer(parse::position);
    cexpr = cexpr->cast(int_type::create());
  }
  if ( !cexpr->isconstant() ){
    using namespace error::decl::initializer::designator;
    not_constant(parse::position);
    return arg.off;
  }
  int nth = cexpr->value();
  map<int, var*> u;
  const type* T = array_type::create(array->element_type(),nth);
  argument tmp(T,u,0,0,0,0,0,0);
  fill_zero(tmp);
  for_each(u.begin(),u.end(),initializer::merge(arg.V,arg.off));
  arg.T = array->element_type();
  arg.off_max = max(arg.off_max, arg.off += tmp.off_max);
  arg.nth_max = max(arg.nth_max, arg.nth = tmp.nth_max);
  return arg.off;
}

int c_compiler::initializer_list::designator_impl::dot(usr* member, initializer::argument& arg)
{
  using namespace std;
  using namespace initializer;
  arg.T = arg.T->unqualified();
  if ( arg.T->m_id != type::RECORD ){
    using namespace error::decl::initializer::designator;
    invalid_dot(parse::position,arg.T);
    return arg.off;
  }
  typedef const record_type REC;
  REC* rec = static_cast<REC*>(arg.T);
  string name = member->m_name;
  pair<int, usr*> ret = rec->offset(name);
  int offset = ret.first;
  member = ret.second;
  if ( !member ){
    using namespace error::decl::initializer::designator;
    not_member(parse::position,name,rec,argument::dst);
    return arg.off;
  }
  map<int, var*> u;
  argument tmp(rec,u,0,0,0,0,0,0);
  fill_zero(tmp);
  u.erase(u.lower_bound(offset),u.end());
  for_each(u.begin(),u.end(),initializer::merge(arg.V,arg.off));
  arg.T = member->m_type;
  arg.off_max = max(arg.off_max, arg.off += offset);
  arg.nth_max = max(arg.nth_max, arg.nth = order(rec,offset));
  return arg.off;
}

const c_compiler::type* c_compiler::initializer_list::complete(const type* T, int offset)
{
  if ( T->m_id != type::ARRAY )
    return T;
  typedef const array_type ARRAY;
  ARRAY* array = static_cast<ARRAY*>(T);
  if ( array->dim() )
    return T;
  T = array->element_type();
  int size = T->size();
  int dim = offset / size;
  if ( offset % size )
    ++dim;
  return array_type::create(T,dim);
}

int c_compiler::initializer::fill_zero(argument& arg)
{
  using namespace std;
  if ( arg.list_pos < 0 )
    return 0;
  if ( arg.list_pos != arg.list_len )
    return 0;
  if ( arg.off_max == arg.T->size() )
    return 0;
  if ( !arg.T->size() )
    return 0;

  while ( 1 ){
    pair<int, const type*> ret = arg.T->current(arg.nth_max);
    if ( ret.first < 0 )
      break;
    arg.off_max = max(arg.off_max, arg.off = ret.first);
    const type* T = ret.second;
    if ( T->m_id == type::BIT_FIELD ){
      typedef const bit_field_type BF;
      BF* bf = static_cast<BF*>(T);
      T = bf->integer_type();
    }
    if ( T->scalar() ){
      var* zero = integer::create(0);
      zero = zero->cast(T);
      arg.V[arg.off] = zero;
      arg.off_max = max(arg.off_max, arg.off += T->size());
    }
    else {
      map<int, var*> u;
      argument tmp(T,u,0,0,0,0,0,0);
      fill_zero(tmp);
      for_each(u.begin(),u.end(),merge(arg.V,arg.off));
      arg.off_max = max(arg.off_max, arg.off += tmp.off);
    }
    arg.nth = ++arg.nth_max;
  }
  return arg.off;
}

int c_compiler::initializer::bit_field(var* y, argument& arg)
{
  using namespace std;
  typedef const record_type REC;
  REC* rec =static_cast<REC*>(arg.T);
  const vector<usr*>& members = rec->member();
  usr* member = members[arg.nth];
  typedef const bit_field_type BF;
  BF* bf = static_cast<BF*>(member->m_type);
  const type* T = bf->integer_type();
  y = y->cast(T);
  usr* a = refbit::mask(bf->bit());
  y = expr::binary('&', y, a);
  int pos = rec->position(member);
  var* p = integer::create(pos);
  y = y->lsh(p);
  var*& x = arg.V[arg.off];
  if ( !x ){
    x = y;
    arg.nth_max = max(arg.nth_max,++arg.nth);
    return arg.off;
  }
  usr* b = refbit::mask(bf->bit(),pos);
  x = expr::binary('&', x, b);
  x = expr::binary('|', x, y);
  arg.nth_max = max(arg.nth_max,++arg.nth);
  const usr::flag_t& flag = member->m_flag;
  if ( flag & usr::MSB_FIELD )
    arg.off_max = max(arg.off_max, arg.off += T->size());
  return arg.off;
}
