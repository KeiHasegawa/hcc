#ifndef _MISC_H_
#define _MISC_H_

namespace c_compiler {

namespace ucn {
  std::string conv(std::string);
} // end of namespace ucn

struct genaddr;

namespace literal {
  extern genaddr* stringa(std::string);
} // end of namespace literal

namespace integer {
  usr* create(std::string);
  usr* create(char);
  usr* create(signed char);
  usr* create(unsigned char);
  usr* create(short int);
  usr* create(unsigned short int);
  usr* create(int);
  usr* create(unsigned int);
  usr* create(long int);
  usr* create(unsigned long int);
  usr* create(__int64);
  usr* create(unsigned __int64);
} // end of namespace integer

namespace SUB_CONST_LONG_impl {
  const type* propagation(const usr* y, const usr* z);
} // end of namespace SUB_CONST_LONG_impl

namespace character {
  usr* create(std::string);
}

namespace floating {
  usr* create(std::string);
  usr* create(float);
  usr* create(double);
  usr* create(long double);
  usr* create(unsigned char*);
} // end of namespace floating

extern std::string new_name(std::string);

namespace pointer {
  template<class V> usr* create(const type* T, V v);
}

namespace conversion {
  const type* arithmetic(var**, var**);
}

namespace expr {
  var* binary(int op, var&, var&);
  var* compound_literal(const type*, parse::initializer_list*);
  var* size(const type*);
  var* cast(const type*, var*);
  var* _va_start(var*, var*);
  var* _va_arg(var*, const type*);
  var* _va_end(var*);
  var* cmp(goto3ac::op, var*, var*);
  struct seq {
    int m_point;
    var* m_var;
    seq(int p, var* v) : m_point(p), m_var(v) {}
  };
  var* logic(bool, seq*, var*);
  var* cond(seq*, seq*, var*);
  namespace assign_impl {
    const type* valid(const type*, var*, bool*);
  }
  extern bool constant_flag;
} // end of namespace expr

namespace decl {
  namespace declarator {
    extern const type* func(const type*, parse::parameter_list* = 0, usr* = 0);
    extern const type* func(const type*, parse::identifier_list*, usr* = 0);
    extern const type* array(const type*, var*, bool, usr* = 0);
    namespace array_impl {
      extern int point;
      extern const type* varray(const type*, var*, usr*);
      extern const type* varray(const type*, var*, const std::vector<tac*>&, usr*);
    }
    extern const type* pointer(const type*, const type*);
  } // end of namespace declarator
  extern bool static_storage_duration(const usr*);
  extern int gen_loff(usr*, std::pair<int,var*>);
  extern void check(var*);
} // end of namespace decl

namespace stmt {
  namespace label {
    void action(usr*);
    struct data_t {
      to3ac* m_to;
      file_t m_file;
      scope* m_scope;
      data_t() : m_to(0), m_scope(0) {}
      data_t(to3ac* a, const file_t& b, scope* c) : m_to(a), m_file(b), m_scope(c) {}
    };
    extern std::map<std::string, data_t> data;
    struct used_t {
      goto3ac* m_goto;
      file_t m_file;
      scope* m_scope;
      used_t(goto3ac* a, const file_t& b, scope* c)
        : m_goto(a), m_file(b), m_scope(c) {}
    };
    extern std::map<std::string, std::vector<used_t> > used;
    extern std::vector<usr*> vm;
  } // end of namespace label
  void _case(var*);
  void _default();
  void expr(var*);
  void if_expr(var*);
  void end_if();
  void else_action();
  void switch_expr(var*);
  void end_switch();
  void while_expr(to3ac*, var*);
  void end_while();
  void _do();
  void end_do(to3ac*, var*);
  void for_expr1(var*);
  extern bool for_decl;
  void for_expr2(var*);
  void for_expr3(var*);
  void end_for(int);
  void _goto(usr*);
  void _continue();
  void _break();
  void _return(var*);
  namespace _asm_ {
          struct operand {
                  var* m_specifier;
                  var* m_expr;
                  operand(var* spec, var* expr) : m_specifier(spec), m_expr(expr) {}
          };
          struct operands : std::vector<operand*> {
            ~operands()
             {
               using namespace std;
               for_each(begin(), end(), deleter<operand>());
             }
          };
          typedef std::vector<var*> reg_list;
          struct operand_list {
                  operands* m_output;
                  operands* m_input;
                  reg_list* m_work;
                  operand_list(operands* output, operands* input, reg_list* work) : m_output(output), m_input(input), m_work(work) {}
                  ~operand_list() { delete m_output; delete m_input; delete m_work; }
          };
          void action(var*);
          void action(var*, operand_list*);
  } // end of namespace _asm_
} // end of namespace stmt

inline int update(goto3ac* go, to3ac* to)
{
  go->m_to = to;
  return 0;
}

extern std::vector<var*> garbage;

namespace type_impl {
  extern void update(int (*)(int));
} // end of namespace type_impl

namespace tac_impl {
  extern void dump(std::ostream&, const tac*);
} // end of namespace tac_impl

namespace scope_impl {
  extern void dump(scope* = &scope::root, int = 0);
} // end of namespace scope_impl

extern void destroy_temporary();

namespace function_definition {
  extern void begin(parse::decl_specs*, usr*);
  extern void action(fundef* fdef, std::vector<tac*>& vcode, bool from_parser);
  extern std::map<std::string, usr*> table;
  namespace static_inline {
    struct info {
      fundef* m_fundef;
      std::vector<tac*> m_code;
      var* m_ret;
      std::vector<tac*> m_expanded;
      block* m_param;
      info(){}
      info(fundef* f, const std::vector<tac*>& c)
        : m_fundef(f), m_code(c), m_ret(0), m_param(0) {}
      ~info();
    };
    namespace expand { extern void action(info*); }
    struct skipped_t : std::map<usr*,info*> {
#ifdef _DEBUG
      ~skipped_t(){ std::for_each(begin(),end(),deleter2<usr*,info>()); }
#endif // _DEBUG
    };
    extern skipped_t skipped;
        namespace just_refed {
                struct info {
                        file_t m_file;
                        usr::flag m_flag;
                        usr* m_func;
                        info(file_t file, usr::flag flag, usr* u)
                                : m_file(file), m_flag(flag), m_func(u) {}
                };
                struct table_t : std::map<std::string, info*> {
#ifdef _DEBUG
                        ~table_t() { std::for_each(begin(), end(), deleter2<std::string, info>()); }
#endif // _DEBUG
                };
                extern table_t table;
                void nodef(const std::pair<std::string, info*>&);
        }  // end of namespace just_refed
        void check_skipped(tac*);
        void gencode(info*);
  } // end of namespace static_inline
  namespace Inline {
    extern std::map<std::string, std::vector<usr*> > decled;
    extern void nodef(const std::pair<std::string, std::vector<usr*> >&);
    struct after : var {
      usr* m_func;
      std::vector<var*>* m_arg;
      scope* m_scope;
      tac* m_point;
      static std::vector<after*> lists;
      after(const type*, usr*, std::vector<var*>*, tac*);
      bool expand(std::string, std::vector<tac*>&);
    };
    namespace resolve {
      extern void action();
      extern bool flag;
    } // end of namespace resolve
  } // end of namespace Inline
  extern void dump(const fundef* fdef, const std::vector<tac*>&);
} // end of namespace function_definition

extern usr* declaration1(usr*, bool);
extern void declaration2(usr*, parse::initializer*);

struct generated : virtual var {
  const type* m_org;
  std::vector<tac*> m_code;
  generated(const pointer_type* G, const type* T)
    : var(G), m_org(T) {}
  generated* generated_cast(){ return this; }
  var* ppmm(bool, bool);
  var* size();
  var* assign(var*);
  ~generated()
  {
    std::for_each(m_code.begin(),m_code.end(),deleter<tac>());
  }
};

#pragma warning ( disable : 4250 )
struct genaddr : generated, addrof {
  var* m_tmp;
  void mark();
  genaddr(const pointer_type*, const type*, var*, int);
  var* rvalue();
  var* subscripting(var*);
  var* call(std::vector<var*>*);
  var* address();
  var* indirection();
  bool lvalue() const { return true; }
  var* offref(const type*, var*);
  var* size(int);
  genaddr* genaddr_cast(){ return this; }
};

struct ref : var {
  const type* m_result;
  ref(const pointer_type*);
  var* rvalue();
  bool lvalue() const { return true; }
  var* address();
  var* ppmm(bool, bool);
  var* assign(var*);
  const type* result_type() const { return m_result; }
};

struct refaddr : ref {
  addrof m_addrof;
  refaddr(const pointer_type* pt, var* v, int offset)
    : ref(pt), m_addrof(pt,v,offset) {}
  var* rvalue();
  bool lvalue() const { return m_addrof.m_ref->lvalue(); }
  var* assign(var*);
  var* address();
  var* offref(const type*, var*);
};

struct refbit : refaddr {
  usr* m_member;
  int m_position;
  int m_bit;
  bool m_dot;
  refbit(const pointer_type* pt, var* ref, int offset, usr* member, int position, int bit, bool dot)
    : refaddr(pt,ref,offset), m_member(member), m_position(position), m_bit(bit), m_dot(dot) {}
  var* rvalue();
  var* assign(var*);
  var* address();
  var* size(int);
  static usr* mask(int);
  static usr* mask(int, int);
};

extern std::vector<tac*> code;

template<class V> struct refimm : ref {
  V m_addr;
  var* common();
  refimm(const pointer_type* pt, V addr) : ref(pt), m_addr(addr) {}
  var* rvalue()
  {
    using namespace std;
    if ( scope::current->m_id == scope::BLOCK ){
      vector<var*>& v = garbage;
      vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
      assert(p != v.rend());
      v.erase(p.base()-1);
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(this);
      code.push_back(new assign3ac(this, common()));
    }
    return ref::rvalue();
  }
  var* address()
  {
    return pointer::create(m_type, m_addr);
  }
  var* assign(var* op)
  {
    using namespace std;
    if ( scope::current->m_id == scope::BLOCK ){
      vector<var*>& v = garbage;
      vector<var*>::reverse_iterator p = find(v.rbegin(),v.rend(),this);
      assert(p != v.rend());
      v.erase(p.base()-1);
      block* b = static_cast<block*>(scope::current);
      b->m_vars.push_back(this);
      code.push_back(new assign3ac(this, common()));
    }
    return ref::assign(op);
  }
 };

struct refsomewhere : ref {
  var* m_ref;
  var* m_offset;
  refsomewhere(const pointer_type* pt, var* r, var* o) : ref(pt), m_ref(r), m_offset(o) {}
  var* rvalue();
  var* address();
  var* assign(var*);
  var* offref(const type*, var*); 
};

struct enum_member : usr {
  usr* m_value;
  enum_member(const usr& u, usr* value) : usr(u), m_value(value) {}
};

struct var01 : var {
  int m_one;
  int m_zero;
  var01(const type* T) : var(T), m_one(-1), m_zero(-1) {}
  void if_expr();
  void else_action();
  void end_if();
  void while_expr(to3ac*);
  void end_while();
  void end_do(to3ac*);
  void for_expr2();
  void end_for(int);
  var* cond(int, int, var*, var*);
  void sweep();
  static void sweep(std::vector<tac*>::iterator);
};

struct log01 : var01 {
  int m_goto1;
  log01(const type* T, int goto1) : var01(T), m_goto1(goto1) {}
  void end_do(to3ac*);
};

struct opposite_t : std::map<goto3ac::op,goto3ac::op> {
  opposite_t();
};

extern opposite_t opposite;

typedef std::pair<const fundef*, std::vector<tac*> > FUNCS_ELEMENT_TYPE;
extern std::vector<std::pair<const fundef*, std::vector<tac*> > > funcs;
inline param_scope* get_pm(FUNCS_ELEMENT_TYPE& elem)
{
  return elem.first->m_param;
}

namespace error {
  enum LANG { jpn, other };
  extern LANG lang;
  namespace cmdline {
    extern void open(std::string);
    extern void generator();
  }
  extern int counter;
  extern bool headered;
  extern void header(const file_t&, std::string);
  namespace parse {
    extern void missing(const file_t&, char);
  }
  extern void undeclared(const file_t&, std::string);
  namespace literal {
    namespace integer {
      extern void too_large(const file_t&, std::string, const type*);
    }
    namespace character {
      extern void invalid(const file_t&, std::string, const type*);
    }
  } // end of namespace literal
  namespace expr {
    namespace underscore_func {
      extern void outside(const file_t&);
      extern void declared(const file_t&);
    }
    namespace subscripting {
      extern void not_pointer(const file_t&, const var*);
      extern void not_object(const file_t&, const type*);
      extern void not_integer(const file_t&, const var*);
    }
    namespace call {
      extern void not_function(const file_t&, const var*);
      extern void num_of_arg(const file_t&, const var*, int, int);
      extern void not_object(const file_t&, const var*);
      extern void mismatch_argument(const file_t&, int, bool, const var*);
    }
    namespace member {
      extern void not_pointer(const usr*, const var*);
      extern void not_record(const usr*, const var*);
      extern void not_member(const usr*, const record_type*, const var*);
    }
    namespace ppmm {
      extern void not_lvalue(const file_t&, bool, const var*);
      extern void not_modifiable(const file_t&, bool, const var*);
      extern void not_modifiable_lvalue(const file_t&, bool, const type*);
      extern void not_scalar(const file_t&, bool, const var*);
      extern void invalid_pointer(const file_t&, bool, const pointer_type*);
    }
    namespace address {
      extern void not_lvalue(const file_t&);
      extern void bit_field(const file_t&, const usr*);
      extern void specified_register(const file_t&, const usr*);
      namespace implicit {
        extern void specified_register(const file_t&, const usr*);
      }
    }
    namespace indirection {
      extern void not_pointer(const file_t&);
    }
    namespace unary {
      extern void invalid(const file_t&, int, const type*);
    }
    namespace size {
      extern void invalid(const file_t&, const type*);
      extern void bit_field(const file_t&, const usr*);
    }
    namespace cast {
      extern void not_scalar(const file_t&);
      extern void invalid(const file_t&);
    }
    namespace va {
      extern void not_lvalue(const file_t&);
      extern void no_size(const file_t&);
      extern void invalid(std::string, const file_t&, const var*);
    }
    namespace binary {
      extern void invalid(const file_t&, int, const type*, const type*);
      extern void not_compatible(const file_t&, const pointer_type*, const pointer_type*);
    }
    namespace cond {
      extern void not_scalar(const file_t&);
      extern void mismatch(const file_t&);
    }
    namespace assign {
      extern void not_modifiable(const file_t&, const usr*);
      extern void not_lvalue(const file_t&);
      extern void not_modifiable_lvalue(const file_t&, const type*);
      extern void invalid(const file_t&, const usr*, bool);
    }
  } // end of namespace expr
  namespace decl {
    extern void empty(const file_t&);
    extern void multiple_type(const file_t&, const type*, const type*);
    extern void invalid_combination(const file_t&, const type*, std::string);
    extern void invalid_combination(const file_t&, std::string, std::string);
    extern void no_type(const usr*);
    extern void no_type(const file_t&);
    extern void redeclaration(const usr*, const usr*, bool);
    extern void redeclaration(const file_t&, const file_t&, std::string);
    extern void not_object(const usr*, const type*);
    namespace storage {
      extern void multiple(const file_t&, usr::flag, usr::flag);
      extern void invalid_function(const usr*);
    }
    namespace struct_or_union {
      extern void incomplete_or_function(const usr*);
      extern void not_ordinary(const usr*);
      namespace bit_field {
        extern void not_integer_bit(const usr*);
        extern void not_constant(const usr*);
        extern void negative(const usr*);
        extern void exceed(const usr*, const type*);
        extern void zero(const usr*);
        extern void not_integer_type(const usr*);
      }
    }
    namespace _enum {
      extern void not_constant(const usr*);
      extern void not_integer(const usr*);
    }
    namespace qualifier {
      extern void invalid(const file_t&, const type*);
    }
    namespace func_spec {
      extern void not_function(const usr*);
      extern void main(const usr*);
      extern void no_definition(const usr*);
      extern void static_storage(const usr*);
      extern void internal_linkage(const file_t&, const usr*);
    }
    namespace declarator {
      namespace func {
        extern void of_func(const file_t&, const usr*);
        extern void of_array(const file_t&, const usr*);
        extern void not_declared(const file_t&, std::string);
        extern void not_parameter(const usr*);
        extern void parameter_omitted(const file_t&, int);
        extern void invalid_storage(const file_t&, const usr*);
        extern void invalid_identifier_list(const file_t&);
      }
      namespace array {
        extern void not_integer(const file_t&, const usr*);
        extern void not_positive(const file_t&, const usr*);
        extern void of_func(const file_t&, const usr*);
        extern void asterisc_dimension(const file_t&, const usr*);
      }
      namespace varray {
        extern void invalid_storage(const usr*);
        extern void initializer(const usr*);
      }
      namespace vm {
        extern void file_scope(const usr*);
        extern void invalid_linkage(const usr*);
      }
    } // end of namespace declarator
    namespace initializer {
      extern void invalid_assign(const file_t&, const usr*, bool);
      extern void not_constant(const file_t&);
      extern void exceed(const usr*);
      extern void not_object(const usr*);
      namespace designator {
        extern void invalid_subscripting(const file_t&, const type*);
        extern void not_integer(const file_t&);
        extern void not_constant(const file_t&);
        extern void invalid_dot(const file_t&, const type*);
        extern void not_member(const file_t&, std::string, const record_type*, const usr*);
      }
      extern void with_extern(const usr*);
    }
    namespace external {
      extern void invalid(const file_t&);
    }
  } // end of namespace decl
  namespace stmt {
    namespace expr {
      extern void incomplete_type(const file_t&);
    }
    namespace label {
      extern void multiple(std::string, const file_t&, const file_t&);
      extern void not_defined(std::string, const file_t&);
    }
    namespace _if {
      extern void not_scalar(const file_t&, const var*);
    }
    namespace _switch {
      extern void not_integer(const file_t&, const var*);
      extern void invalid(bool, const file_t&, const usr*);
    }
    namespace _case {
      extern void not_constant(const file_t&, const var*);
      extern void not_integer(const file_t&, const var*);
      extern void no_switch(const file_t&);
      extern void duplicate(const file_t&, const file_t&);
    }
    namespace _default {
      extern void no_switch(const file_t&);
      extern void multiple(const file_t&, const file_t&);
    }
    namespace _while {
      extern void not_scalar(const file_t&);
    }
    namespace _for {
      extern void not_scalar(const file_t&);
      extern void invalid_storage(const usr*);
    }
    namespace do_while {
      extern void not_scalar(const file_t&);
    }
    namespace _goto {
      extern void invalid(const file_t&, const file_t&, const usr*);
    }
    namespace _break {
      extern void not_within(const file_t&);
    }
    namespace _continue {
      extern void not_within(const file_t&);
    }
    namespace _return {
      extern void invalid(const file_t&, const type*, const type*);
    }
  } // end of namespace stmt
  namespace extdef {
    extern void invalid_storage(const file_t&);
    namespace fundef {
      extern void multiple(const file_t&, const usr*);
      extern void invalid_return(const usr*, const type*);
      extern void invalid_storage(const usr*);
      extern void invalid_initializer(const usr*);
      extern void nodef(const file_t&, std::string, const file_t&);
      extern void typedefed(const file_t&);
    }
  } // end of namespace extdef
} // end of namespace error

namespace warning {
  extern int counter;
  namespace cmdline {
    extern void option(std::string);
    extern void input(std::string);
    extern void generator_option();
    extern void generator_option_option(std::string);
    extern void o_option();
    extern void lang_option(std::string);
    extern void lang_option();
    extern void optimize_option();
  }
  namespace generator {
    extern void open(std::string);
    extern void seed(std::string);
    extern void seed(std::string, std::pair<int,int>);
    extern void option(std::string);
    extern void option(std::string, int, std::string);
    // extern void open_file(std::string);
    extern void generate(std::string);
    // extern void close_file(std::string);
  }
  namespace expr {
    namespace call {
      extern void implicit(const usr*);
    } // end of namespace call
  }
  namespace decl {
    namespace initializer {
      extern void with_extern(const usr*);
    }
  }
  extern void zero_division(const file_t&);
} // end of namespace warning

namespace cmdline {
  extern void setup(int, char**);
  extern std::string prog;
  extern std::string input;
  extern std::string output;
  extern bool output_medium;
  extern std::string generator;
  extern std::vector<std::string> g_generator_option;
  extern int optimize_level;
  extern bool simple_medium;
  extern bool bb_optimize;
  extern bool dag_optimize;
  extern bool output_optinfo;
} // end of namespace cmdline

namespace generator {
  extern void initialize();
  extern void (*generate)(const interface_t*);
  extern int (*close_file)();
  extern void terminate();
  extern long_double_t* long_double;
  extern type::id sizeof_type;
  namespace wchar {
    extern type::id id;
    extern const type* type;
  } // end of namespace wchar
  extern void (*last)(const last_interface_t*);
} // end of namespace generator

namespace initializer {
  using namespace std;
  struct argument {
    static usr* dst;
    const type* T;
    map<int, var*>& V;
    int off;
    int off_max;
    int nth;
    int nth_max;
    int list_pos;
    int list_len;
    argument(const type* t, map<int,var*>& v, int o, int omax, int n, int nmax, int pos, int len)
      : T(t), V(v), off(o), off_max(omax), nth(n), nth_max(nmax), list_pos(pos), list_len(len) {}
  };
  int eval(parse::initializer*, argument&);
  int fill_zero(initializer::argument&);
} // end of namespace initializer

namespace initializer_list {
  struct eval {
    initializer::argument& arg;
    eval(initializer::argument& a) : arg(a) {}
    int operator()(parse::ppair<parse::designation*, parse::initializer*>*);
  };
} // end of namespace initializer_list

namespace optimize {
  extern void action(const fundef* fdef, std::vector<tac*>& v);
  extern void remember_action(const std::vector<tac*>&);
  extern void mark(usr*);
} // end of namespace optimize

namespace names {
  void reset();
} // end of namespace names

namespace live_var {
  using namespace std;
  void dump(string, const map<optimize::basic_block::info*, set<var*> >&);
} // end of namespace live_var

namespace character_impl {
  usr* integer_create(bool wide, char prev, int c);
  usr* integer_create(bool wide, int c);
} // end of namespace character_impl

#ifdef USE_PROFILE
namespace simple_profiler {
  struct info {
    std::string m_label;
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_end;
  };
  struct sentry {
    int m_id;
    static int counter;
    sentry(std::string);
    ~sentry();
    static std::vector<info> all;
    static void output();
  };
} // end of namespace simple_profiler
#endif // USE_PROFILE

} // end of namespace c_compiler

#endif // _MISC_H_
