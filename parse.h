#ifndef _PARSE_H_
#define _PARSE_H_

namespace c_compiler {

template<class C> class pvector : public std::vector<C*> {
public:
  ~pvector(){ for (auto p : *this) delete p; }
};

} // end of namespace c_compiler

namespace c_compiler { namespace parse {
 
namespace identifier {
  extern int judge(std::string);
  namespace judge_impl {
    extern int prev;
  }
}

extern var* string_concatenation(var*, var*);

struct initializer;

struct designator;

typedef pvector<designator> designation;

template<class X, class Y> struct ppair : std::pair<X,Y> {
  ppair(X x, Y y) : std::pair<X,Y>(x,y) {}
  ~ppair()
  {
    delete std::pair<X,Y>::first;
    delete std::pair<X,Y>::second;
  }
};

typedef pvector<ppair<designation*,initializer*> > initializer_list;

struct initializer : std::pair<var*, initializer_list*> {
  initializer(var* a, initializer_list* b) : std::pair<var*, initializer_list*>(a,b) {}
  ~initializer(){ delete second; }
};

typedef std::pair<int, const type*> type_specifier;

struct decl_specs : pvector<type_specifier> {
  static std::vector<int> m_curr;
  static std::vector<int> m_temp;
  static std::stack<decl_specs*> m_stack;
  decl_specs()
  {
    m_curr.clear();
    m_stack.push(this);
  }
  ~decl_specs()
  {
    m_stack.pop();
  }
};

typedef std::vector<usr*> struct_declarator_list;

typedef std::vector<usr*> struct_declaration_list;

struct_declaration_list* struct_declaration(decl_specs*, struct_declarator_list*);

namespace enumerator {
  extern void action(usr*, var*);
  extern usr* prev;
} // end of namespace enumerator

typedef std::vector<int> type_qualifier_list;

typedef pvector<var> identifier_list;

struct designator : std::pair<var*, usr*> {
  designator(var* a, usr* b) : std::pair<var*, usr*>(a,b) {}
  ~designator()
  { delete second; }
};

typedef pvector<designator> designator_list;

typedef std::vector<const type*> parameter_list;

extern const type* pointer(type_qualifier_list*);

namespace parameter {
  extern void enter();
  extern void leave();
  extern void enter2();
  extern bool old_style;
  extern int depth;
}

extern const type* parameter_declaration(decl_specs*, usr*);

extern const type* parameter_declaration(decl_specs*, const type*);

namespace block {
  extern void enter();
  extern void leave();
}

typedef std::vector<var*> argument_expression_list;

extern bool is_last_decl;

extern file_t position;

tag* tag_begin(tag::kind_t, usr*);

const type* struct_or_union_specifier(tag*, struct_declaration_list*);

const type* bit_field(const type*, var*, usr*);

const type* enum_specifier(tag*);

const type* tag_type(int, tag*);

const type* type_name(decl_specs*, const type*);

#ifdef _DEBUG
extern void delete_buffer();
#endif // _DEBUG

extern void unputer(std::string);

extern int guess(int);

} } // end of namespace parse and c_compiler

#endif // _PARSE_H_
