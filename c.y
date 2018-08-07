%{
#include "c_core.h"
#include "parse.h"
#include "misc.h"
%}

%token TYPEDEF_KW EXTERN_KW STATIC_KW REGISTER_KW AUTO_KW
%token INLINE_KW
%token VOID_KW CHAR_KW INT_KW FLOAT_KW DOUBLE_KW
%token SHORT_KW LONG_KW
%token SIGNED_KW UNSIGNED_KW
%token TAG_NAME_LEX TYPEDEF_NAME_LEX
%token CONST_KW VOLATILE_KW RESTRICT_KW

%token STRUCT_KW UNION_KW ENUM_KW

%token CONTINUE_KW CASE_KW SWITCH_KW  FOR_KW
%token WHILE_KW BREAK_KW GOTO_KW DO_KW
%token SIZEOF_KW IF_KW RETURN_KW ELSE_KW DEFAULT_KW

%token INTEGER_CONSTANT_LEX CHARACTER_CONSTANT_LEX FLOATING_CONSTANT_LEX STRING_LITERAL_LEX
%token IDENTIFIER_LEX

%token ADD_ASSIGN_MK OROR_MK GREATEREQ_MK SUB_ASSIGN_MK ANDAND_MK
%token LSH_ASSIGN_MK NOTEQ_MK RSH_ASSIGN_MK
%token LESSEQ_MK PLUSPLUS_MK MUL_ASSIGN_MK DOTS_MK
%token RSH_MK AND_ASSIGN_MK DIV_ASSIGN_MK XOR_ASSIGN_MK
%token MOD_ASSIGN_MK EQUAL_MK MINUSMINUS_MK OR_ASSIGN_MK ARROW_MK LSH_MK

%token BUILTIN_VA_ARG BUILTIN_VA_START BUILTIN_VA_END ASM_KW

%union {
  int m_ival;
  c_compiler::tag::kind m_kind;
  const c_compiler::type* m_type;
  c_compiler::var* m_var;
  c_compiler::usr* m_usr;
  c_compiler::tag* m_tag;
  c_compiler::parse::argument_expression_list* m_argument_expression_list;
  c_compiler::parse::type_specifier* m_type_specifier;
  c_compiler::parse::struct_declaration_list* m_struct_declaration_list;
  c_compiler::parse::struct_declaration_list* m_struct_declaration;
  c_compiler::parse::decl_specs* m_specifier_qualifier_list;
  c_compiler::parse::struct_declarator_list* m_struct_declarator_list;
  c_compiler::parse::decl_specs* m_declaration_specifiers;
  c_compiler::parse::initializer* m_initializer;
  c_compiler::parse::type_qualifier_list* m_type_qualifier_list;
  c_compiler::parse::identifier_list* m_identifier_list;
  c_compiler::parse::designation* m_designation;
  c_compiler::parse::designator_list* m_designator_list;
  c_compiler::parse::designator* m_designator;
  c_compiler::parse::parameter_list* m_parameter_list;
  c_compiler::parse::initializer_list* m_initializer_list;
  c_compiler::expr::seq* m_seq;
  c_compiler::to3ac* m_to3ac;
  c_compiler::stmt::_asm_::operand_list* m_operand_list;
  c_compiler::stmt::_asm_::operands* m_operands;
  c_compiler::stmt::_asm_::operand* m_operand;
  c_compiler::stmt::_asm_::reg_list* m_reg_list;
}

%type<m_ival> storage_class_specifier
%type<m_ival> function_specifier type_qualifier for_expr1 ANDAND_MK OROR_MK '?' ':'
%type<m_ival> SIZEOF_KW
%type<m_kind> struct_or_union
%type<m_var> primary_expression assignment_expression postfix_expression unary_expression
%type<m_var> cast_expression multiplicative_expression additive_expression shift_expression
%type<m_var> relational_expression equality_expression AND_expression exclusive_OR_expression
%type<m_var> inclusive_OR_expression logical_AND_expression logical_OR_expression conditional_expression
%type<m_seq> logical_AND1st logical_OR1st cond1st cond2nd
%type<m_var> constant_expression expression string_literal STRING_LITERAL_LEX
%type<m_usr> IDENTIFIER_LEX INTEGER_CONSTANT_LEX CHARACTER_CONSTANT_LEX FLOATING_CONSTANT_LEX
%type<m_usr> TYPEDEF_NAME_LEX
%type<m_usr> declarator direct_declarator struct_declarator constant
%type<m_type> struct_or_union_specifier enum_specifier parameter_declaration pointer type_name
%type<m_type> abstract_declarator
%type<m_type> direct_abstract_declarator
%type<m_argument_expression_list> argument_expression_list
%type<m_type_specifier> type_specifier
%type<m_struct_declaration_list> struct_declaration_list
%type<m_struct_declaration> struct_declaration
%type<m_specifier_qualifier_list> specifier_qualifier_list
%type<m_struct_declarator_list> struct_declarator_list
%type<m_declaration_specifiers> declaration_specifiers
%type<m_initializer> initializer
%type<m_type_qualifier_list> type_qualifier_list
%type<m_identifier_list> identifier_list
%type<m_designation> designation
%type<m_designator_list> designator_list
%type<m_designator> designator
%type<m_parameter_list> parameter_list parameter_type_list
%type<m_tag> struct_or_union_specifier_begin enum_specifier_begin TAG_NAME_LEX
%type<m_initializer_list> initializer_list
%type<m_to3ac> while
%type<m_operand_list> asm_operand_list
%type<m_operands> asm_operands
%type<m_operand> asm_operand
%type<m_reg_list> reg_list

%start translation_unit

%%

primary_expression
  : IDENTIFIER_LEX { $$ = $1; }
  | constant { $$ = $1; }
  | string_literal
  | '(' expression ')'  { $$ = $2; }
  ;

constant
  : INTEGER_CONSTANT_LEX
  | CHARACTER_CONSTANT_LEX
  | FLOATING_CONSTANT_LEX
  ;

string_literal
  : STRING_LITERAL_LEX
  | string_literal STRING_LITERAL_LEX  { $$ = c_compiler::parse::string_concatenation($1,$2); }
  ;

postfix_expression
  : primary_expression
  | postfix_expression '[' expression ']'  { $$ = $1->subscripting($3); }
  | postfix_expression '('                          ')'  { $$ = $1->call(0); }
  | postfix_expression '(' argument_expression_list ')'  { $$ = $1->call($3); }
  | postfix_expression '.' IDENTIFIER_LEX  { $$ = $1->member($3,true); }
  | postfix_expression ARROW_MK IDENTIFIER_LEX  { $$ = $1->member($3,false); }
  | postfix_expression PLUSPLUS_MK  { $$ = $1->ppmm(true,true); }
  | postfix_expression MINUSMINUS_MK  { $$ = $1->ppmm(false,true); }
  | '(' type_name ')' '{' initializer_list '}'  { $$ = c_compiler::expr::compound_literal($2,$5); }
  | '(' type_name ')' '{' initializer_list ',' '}'  { $$ = c_compiler::expr::compound_literal($2,$5); }
  ;

argument_expression_list
  : assignment_expression                               { $$ = new std::vector<c_compiler::var*>; $$->push_back($1); }
  | argument_expression_list ',' assignment_expression  { $$ = $1; $$->push_back($3); }
  ;

unary_expression
  : postfix_expression
  | PLUSPLUS_MK unary_expression    { $$ = $2->ppmm(true,false); }
  | MINUSMINUS_MK unary_expression  { $$ = $2->ppmm(false,false); }
  | '&' cast_expression             { $$ = $2->address(); }
  | '*' cast_expression             { $$ = $2->indirection(); }
  | '+' cast_expression             { $$ = $2->plus(); }
  | '-' cast_expression             { $$ = $2->minus(); }
  | '~' cast_expression             { $$ = $2->tilde(); }
  | '!' cast_expression             { $$ = $2->_not(); }
  | SIZEOF_KW unary_expression      { $$ = $2->size($1); }
  | SIZEOF_KW '(' type_name ')'     { $$ = c_compiler::expr::size($3); }
  ;

cast_expression
  : unary_expression
  | '(' type_name ')' cast_expression  { $$ = c_compiler::expr::cast($2,$4); }
  | BUILTIN_VA_START '(' cast_expression ',' cast_expression ')' { $$ = c_compiler::expr::_va_start($3,$5); }
  | BUILTIN_VA_ARG '(' cast_expression ',' type_name ')' { $$ = c_compiler::expr::_va_arg($3,$5); }
  | BUILTIN_VA_END '(' cast_expression ')' { $$ = c_compiler::expr::_va_end($3); }
  ;

multiplicative_expression
  : cast_expression
  | multiplicative_expression '*' cast_expression  { $$ = $1->mul($3); }
  | multiplicative_expression '/' cast_expression  { $$ = $1->div($3); }
  | multiplicative_expression '%' cast_expression  { $$ = $1->mod($3); }
  ;

additive_expression
  : multiplicative_expression
  | additive_expression '+' multiplicative_expression  { $$ = $1->add($3); }
  | additive_expression '-' multiplicative_expression  { $$ = $1->sub($3); }
  ;

shift_expression
  : additive_expression
  | shift_expression LSH_MK additive_expression  { $$ = $1->lsh($3); }
  | shift_expression RSH_MK additive_expression  { $$ = $1->rsh($3); }
  ;

relational_expression
  : shift_expression
  | relational_expression '<' shift_expression           { $$ = $1->lt($3); }
  | relational_expression '>' shift_expression           { $$ = $1->gt($3); }
  | relational_expression LESSEQ_MK shift_expression     { $$ = $1->le($3); }
  | relational_expression GREATEREQ_MK shift_expression  { $$ = $1->ge($3); }
  ;

equality_expression
  : relational_expression
  | equality_expression EQUAL_MK relational_expression  { $$ = $1->eq($3); }
  | equality_expression NOTEQ_MK relational_expression  { $$ = $1->ne($3); }
  ;

AND_expression
  : equality_expression
  | AND_expression '&' equality_expression  { $$ = $1->bit_and($3); }
  ;

exclusive_OR_expression
  : AND_expression
  | exclusive_OR_expression '^' AND_expression  { $$ = $1->bit_xor($3); }
  ;

inclusive_OR_expression
  : exclusive_OR_expression
  | inclusive_OR_expression '|' exclusive_OR_expression  { $$ = $1->bit_or($3); }
  ;

logical_AND_expression
  : inclusive_OR_expression
  | logical_AND1st inclusive_OR_expression  { $$ = c_compiler::expr::logic(true,$1,$2); }
  ;

logical_AND1st
  : logical_AND_expression ANDAND_MK { using namespace c_compiler; $$ = new expr::seq(code.size(),$1); }
  ;

logical_OR_expression
  : logical_AND_expression
  | logical_OR1st logical_AND_expression  { $$ = c_compiler::expr::logic(false,$1,$2); }
  ;

logical_OR1st
  : logical_OR_expression OROR_MK { using namespace c_compiler; $$ = new expr::seq(code.size(),$1); }
  ;

conditional_expression
  : logical_OR_expression
  | cond1st cond2nd conditional_expression  { $$ = c_compiler::expr::cond($1,$2,$3); }
  ;

cond1st
  : logical_OR_expression '?'  { using namespace c_compiler; $$ = new expr::seq(code.size(),$1); }
  ;

cond2nd
  : expression ':'  { using namespace c_compiler; $$ = new expr::seq(code.size(),$1); }
  ;

assignment_expression
  : conditional_expression
  | unary_expression      '='      assignment_expression  { $$ = $1->assign($3); }
  | unary_expression MUL_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->mul($3)); }
  | unary_expression DIV_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->div($3)); }
  | unary_expression MOD_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->mod($3)); }
  | unary_expression ADD_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->add($3)); }
  | unary_expression SUB_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->sub($3)); }
  | unary_expression LSH_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->lsh($3)); }
  | unary_expression RSH_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->rsh($3)); }
  | unary_expression AND_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->bit_and($3)); }
  | unary_expression XOR_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->bit_xor($3)); }
  | unary_expression  OR_ASSIGN_MK assignment_expression  { $$ = $1->assign($1->bit_or($3)); }
  ;

expression
  : assignment_expression
  | expression ',' assignment_expression  { $$ = $1->comma($3); }
  ;

constant_expression
  : conditional_expression
  ;

declaration
  : declaration_specifiers                      ';'  { c_compiler::declaration1(0,false); delete c_compiler::parse::decl_specs::m_stack.top(); }
  | declaration_specifiers init_declarator_list ';'  { delete c_compiler::parse::decl_specs::m_stack.top(); }
  ;

declaration_specifiers
  : storage_class_specifier                         { using namespace c_compiler::parse; $$ = new decl_specs; $$->push_back(new type_specifier($1,0)); }
  | storage_class_specifier declaration_specifiers  { using namespace c_compiler::parse; $$ = $2; $$->push_back(new type_specifier($1,0)); }
  | type_specifier                                  { using namespace c_compiler::parse; $$ = new decl_specs; $$->push_back($1); }
  | type_specifier declaration_specifiers           { using namespace c_compiler::parse; $$ = $2; $$->push_back($1); }
  | type_qualifier                                  { using namespace c_compiler::parse; $$ = new decl_specs; $$->push_back(new type_specifier($1,0)); }
  | type_qualifier declaration_specifiers           { using namespace c_compiler::parse; $$ = $2; $$->push_back(new type_specifier($1,0)); }
  | function_specifier                              { using namespace c_compiler::parse; $$ = new decl_specs; $$->push_back(new type_specifier($1,0)); }
  | function_specifier declaration_specifiers       { using namespace c_compiler::parse; $$ = $2; $$->push_back(new type_specifier($1,0)); }
  ;

init_declarator_list
  : init_declarator
  | init_declarator_list ',' init_declarator
  ;

init_declarator
  : declarator     { c_compiler::declaration1($1,false); }
  | declarator '=' { $1 = c_compiler::declaration1($1,true); c_compiler::parse::decl_specs::m_stack.push(0); } initializer  { c_compiler::parse::decl_specs::m_stack.pop(); c_compiler::declaration2($1,$4); }
  ;

storage_class_specifier
  : TYPEDEF_KW   { c_compiler::parse::decl_specs::m_curr.push_back($$ = TYPEDEF_KW); }
  | EXTERN_KW    { c_compiler::parse::decl_specs::m_curr.push_back($$ = EXTERN_KW); }
  | STATIC_KW    { c_compiler::parse::decl_specs::m_curr.push_back($$ = STATIC_KW); }
  | AUTO_KW      { c_compiler::parse::decl_specs::m_curr.push_back($$ = AUTO_KW); }
  | REGISTER_KW  { c_compiler::parse::decl_specs::m_curr.push_back($$ = REGISTER_KW); }
  ;

type_specifier
  : VOID_KW                    { $$ = new c_compiler::parse::type_specifier(VOID_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(VOID_KW); }
  | CHAR_KW                    { $$ = new c_compiler::parse::type_specifier(CHAR_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(CHAR_KW); }
  | SHORT_KW                   { $$ = new c_compiler::parse::type_specifier(SHORT_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(SHORT_KW); }
  | INT_KW                     { $$ = new c_compiler::parse::type_specifier(INT_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(INT_KW); }
  | LONG_KW                    { $$ = new c_compiler::parse::type_specifier(LONG_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(LONG_KW); }
  | FLOAT_KW                   { $$ = new c_compiler::parse::type_specifier(FLOAT_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(FLOAT_KW); }
  | DOUBLE_KW                  { $$ = new c_compiler::parse::type_specifier(DOUBLE_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(DOUBLE_KW); }
  | SIGNED_KW                  { $$ = new c_compiler::parse::type_specifier(SIGNED_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(SIGNED_KW); }
  | UNSIGNED_KW                { $$ = new c_compiler::parse::type_specifier(UNSIGNED_KW,0); c_compiler::parse::decl_specs::m_curr.push_back(UNSIGNED_KW); }
  | struct_or_union_specifier  { $$ = new c_compiler::parse::type_specifier(0,$1); }
  | enum_specifier             { $$ = new c_compiler::parse::type_specifier(0,$1); }
  | TYPEDEF_NAME_LEX           { $$ = new c_compiler::parse::type_specifier(0,$1->m_type); c_compiler::parse::decl_specs::m_curr.push_back(TYPEDEF_NAME_LEX); }
  ;

struct_or_union_specifier
  : struct_or_union_specifier_begin struct_declaration_list '}'  { using namespace c_compiler::parse; $$ = struct_or_union_specifier($1,$2); c_compiler::parse::decl_specs::m_curr.push_back(TAG_NAME_LEX); }
  | struct_or_union IDENTIFIER_LEX                               { $$ = c_compiler::parse::tag_begin($1,$2)->m_types.first; }
  | struct_or_union TAG_NAME_LEX                                 { $$ = c_compiler::parse::tag_type($1,$2); }
  ;

struct_or_union_specifier_begin
  : struct_or_union IDENTIFIER_LEX '{' { using namespace c_compiler::parse; $$ = tag_begin($1,$2); decl_specs::m_curr.clear(); }
  | struct_or_union                '{' { using namespace c_compiler::parse; $$ = tag_begin($1,static_cast<c_compiler::usr*>(0)); decl_specs::m_curr.clear(); }
  ;

struct_or_union
  : STRUCT_KW  { $$ = c_compiler::tag::STRUCT; c_compiler::parse::decl_specs::m_curr.push_back(STRUCT_KW); }
  | UNION_KW   { $$ = c_compiler::tag::UNION;  c_compiler::parse::decl_specs::m_curr.push_back(UNION_KW); }
  ;

struct_declaration_list
  : struct_declaration { $$ = $1; }
  | struct_declaration_list struct_declaration  { $$ = $1; std::copy($2->begin(),$2->end(),std::back_inserter(*$1)); delete $2; }
  ;

struct_declaration
  : specifier_qualifier_list struct_declarator_list ';'  { using namespace c_compiler::parse; $$ = struct_declaration($1,$2); }
  | specifier_qualifier_list                        ';'  { using namespace c_compiler::parse; $$ = struct_declaration($1,0);  }
  ;

specifier_qualifier_list
  : type_specifier                           { $$ = new c_compiler::parse::decl_specs; $$->push_back($1); }
  | type_specifier specifier_qualifier_list  { $$ = $2; $$->push_back($1); }
  | type_qualifier                           { using namespace c_compiler::parse; $$ = new decl_specs; $$->push_back(new type_specifier($1,0)); }
  | type_qualifier specifier_qualifier_list  { $$ = $2; $$->push_back(new c_compiler::parse::type_specifier($1,0)); }
  ;

struct_declarator_list
  : struct_declarator                             { $$ = new c_compiler::parse::struct_declarator_list; $$->push_back($1); }
  | struct_declarator_list ',' struct_declarator  { $$ = $1; $$->push_back($3); }
  ;

struct_declarator
  : declarator
  |            ':' { using namespace c_compiler; expr::constant_flag = true; parse::decl_specs::m_stack.push(0); } constant_expression  { using namespace c_compiler; parse::decl_specs::m_stack.pop(); expr::constant_flag = false; $$ = new usr("",backpatch_type::create(),usr::NONE,parse::position); $$->m_type = parse::bit_field($$->m_type,$3,$$); }
  | declarator ':' { using namespace c_compiler; expr::constant_flag = true; parse::decl_specs::m_stack.push(0); } constant_expression  { using namespace c_compiler; parse::decl_specs::m_stack.pop(); expr::constant_flag = false; $$ = $1; $$->m_type = parse::bit_field($$->m_type,$4,$$); }
  ;

enum_specifier
  : enum_specifier_begin enumerator_list '}'      { $$ = c_compiler::parse::enum_specifier($1); }
  | enum_specifier_begin enumerator_list ',' '}'  { $$ = c_compiler::parse::enum_specifier($1); }
  | ENUM_KW IDENTIFIER_LEX                        { using namespace c_compiler; $$ = parse::tag_begin(tag::ENUM,$2)->m_types.first; }
  | ENUM_KW TAG_NAME_LEX                          { using namespace c_compiler; $$ = parse::tag_type(tag::ENUM,$2); }
  ;

enum_specifier_begin
  : ENUM_KW IDENTIFIER_LEX '{'  { using namespace c_compiler; $$ = parse::tag_begin(tag::ENUM,$2); parse::decl_specs::m_curr.push_back(TAG_NAME_LEX); }
  | ENUM_KW                '{'  { using namespace c_compiler; $$ = parse::tag_begin(tag::ENUM,static_cast<usr*>(0)); parse::decl_specs::m_curr.push_back(TAG_NAME_LEX); }
  ;

enumerator_list
  : enumerator
  | enumerator_list ',' enumerator
  ;

enumerator
  : IDENTIFIER_LEX { c_compiler::parse::enumerator::action($1,0); }
  | IDENTIFIER_LEX '='
    {
      using namespace c_compiler;
      expr::constant_flag = true;
      parse::decl_specs::m_temp = parse::decl_specs::m_curr;
      parse::decl_specs::m_curr.clear();
    }
    constant_expression
    {
      using namespace std;
      using namespace c_compiler;
      parse::decl_specs::m_curr = parse::decl_specs::m_temp;
      expr::constant_flag = false;
      parse::enumerator::action($1,$4);
    }
  ;

type_qualifier
  : CONST_KW     { c_compiler::parse::decl_specs::m_curr.push_back($$ = CONST_KW); }
  | RESTRICT_KW  { c_compiler::parse::decl_specs::m_curr.push_back($$ = RESTRICT_KW); }
  | VOLATILE_KW  { c_compiler::parse::decl_specs::m_curr.push_back($$ = VOLATILE_KW); }
  ;

function_specifier
  : INLINE_KW  { $$ = INLINE_KW; }
  ;

declarator
  :         direct_declarator
  | pointer direct_declarator  { $$ = $2; $$->m_type = c_compiler::decl::declarator::pointer($1,$2->m_type); }
  ;

direct_declarator
  : IDENTIFIER_LEX
  | '(' declarator ')'                               { $$ = $2; }
  | direct_declarator begin_array end_array                        { $$ = $1; $$->m_type = c_compiler::decl::declarator::array($1->m_type,0,false,$1); }
  | direct_declarator begin_array assignment_expression end_array  { $$ = $1; $$->m_type = c_compiler::decl::declarator::array($1->m_type,$3,false,$1); }
  | direct_declarator begin_array '*' end_array                    { $$ = $1; $$->m_type = c_compiler::decl::declarator::array($1->m_type,0,true,$1); }
  | direct_declarator '(' enter_parameter parameter_type_list leave_parameter ')'    { $$ = $1; $$->m_type = c_compiler::decl::declarator::func($1->m_type,$4,$1); }
  | direct_declarator '(' enter_parameter                 leave_parameter ')'    { $$ = $1; $$->m_type = c_compiler::decl::declarator::func($1->m_type,static_cast<c_compiler::parse::parameter_list*>(0),$1); }
  | direct_declarator '(' enter_parameter identifier_list leave_parameter ')'    { $$ = $1; $$->m_type = c_compiler::decl::declarator::func($1->m_type,$4,$1); }
  ;

begin_array
  : '[' { c_compiler::parse::decl_specs::m_stack.push(0); c_compiler::decl::declarator::array_impl::point = c_compiler::code.size(); }
  ;

end_array
  : ']' { c_compiler::parse::decl_specs::m_stack.pop(); }
  ;

pointer
  : '*'                              { using namespace c_compiler; $$ = parse::pointer(0); }
  | '*' type_qualifier_list          { using namespace c_compiler; $$ = parse::pointer($2); }
  | '*'                     pointer  { using namespace c_compiler; $$ = pointer_type::create($2); }
  | '*' type_qualifier_list pointer  { using namespace c_compiler; $$ = pointer_type::create(parse::pointer($2)); }
  ;

type_qualifier_list
  : type_qualifier                      { $$ = new c_compiler::parse::type_qualifier_list; $$->push_back($1); }
  | type_qualifier_list type_qualifier  { $$ = $1; $$->push_back($2); }
  ;

parameter_type_list
  : parameter_list
  | parameter_list ',' DOTS_MK  { $$ = $1; $$->push_back(c_compiler::ellipsis_type::create()); }
  ;

parameter_list
  : parameter_declaration                     { $$ = new c_compiler::parse::parameter_list; $$->push_back($1); }
  | parameter_list ',' parameter_declaration  { $$ = $1; $$->push_back($3); }
  ;

parameter_declaration
  : declaration_specifiers declarator           { $$ = c_compiler::parse::parameter_declaration($1,$2); }
  | declaration_specifiers                      { $$ = c_compiler::parse::parameter_declaration($1,static_cast<c_compiler::usr*>(0)); }
  | declaration_specifiers abstract_declarator  { $$ = c_compiler::parse::parameter_declaration($1,$2); }
  ;

identifier_list
  : IDENTIFIER_LEX                      { $$ = new c_compiler::parse::identifier_list; $$->push_back($1); }
  | identifier_list ',' IDENTIFIER_LEX  { $$ = $1; $$->push_back($3); }
  ;

type_name
  : specifier_qualifier_list                      { $$ = c_compiler::parse::type_name($1,0); }
  | specifier_qualifier_list abstract_declarator  { $$ = c_compiler::parse::type_name($1,$2); }
  ;

abstract_declarator
  : pointer
  |         direct_abstract_declarator
  | pointer direct_abstract_declarator  { $$ = $2; $$ = c_compiler::decl::declarator::pointer($1,$2); }
  ;

direct_abstract_declarator
  : '(' abstract_declarator ')'                               { $$ = $2; }
  |                            '['                       ']'  { using namespace c_compiler; $$ = decl::declarator::array(backpatch_type::create(),0,false); }
  |                            '[' assignment_expression ']'  { using namespace c_compiler; $$ = decl::declarator::array(backpatch_type::create(),$2,false); }
  | direct_abstract_declarator '['                       ']'  { using namespace c_compiler; $$ = decl::declarator::array($1,0,false); }
  | direct_abstract_declarator '[' assignment_expression ']'  { using namespace c_compiler; $$ = decl::declarator::array($1,$3,false); }
  |                            '[' '*' ']'                    { using namespace c_compiler; $$ = decl::declarator::array(backpatch_type::create(),0,true); }
  | direct_abstract_declarator '[' '*' ']'                    { using namespace c_compiler; $$ = decl::declarator::array($1,0,true); }
  |                            '('                     ')'    { using namespace c_compiler; $$ = decl::declarator::func(backpatch_type::create()); }
  |                            '(' enter_parameter parameter_type_list leave_parameter ')'    { using namespace c_compiler; $$ = decl::declarator::func(backpatch_type::create(),$3); }
  | direct_abstract_declarator '('                     ')'    { using namespace c_compiler; $$ = decl::declarator::func($1); }
  | direct_abstract_declarator '(' enter_parameter parameter_type_list leave_parameter ')'    { using namespace c_compiler; $$ = decl::declarator::func($1,$4); }
  ;

initializer
  : assignment_expression         { $$ = new c_compiler::parse::initializer($1,0); }
  | '{' initializer_list '}'      { $$ = new c_compiler::parse::initializer(0,$2); }
  | '{' initializer_list ',' '}'  { $$ = new c_compiler::parse::initializer(0,$2); }
  ;

initializer_list
  :             initializer                       { $$ = new c_compiler::parse::initializer_list; $$->push_back(new c_compiler::parse::ppair<c_compiler::parse::designation*,c_compiler::parse::initializer*>(0,$1)); }
  | designation initializer                       { $$ = new c_compiler::parse::initializer_list; $$->push_back(new c_compiler::parse::ppair<c_compiler::parse::designation*,c_compiler::parse::initializer*>($1,$2)); }
  | initializer_list ','             initializer  { $$ = $1; $$->push_back(new c_compiler::parse::ppair<c_compiler::parse::designation*,c_compiler::parse::initializer*>(0,$3)); }
  | initializer_list ',' designation initializer  { $$ = $1; $$->push_back(new c_compiler::parse::ppair<c_compiler::parse::designation*,c_compiler::parse::initializer*>($3,$4)); }
  ;

designation
  : designator_list '=' { $$ = $1; }
  ;

designator_list
  : designator                  { $$ = new c_compiler::parse::designator_list; $$->push_back($1); }
  | designator_list designator  { $$ = $1; $$->push_back($2); }
  ;

designator
  : '[' { c_compiler::expr::constant_flag = true; } constant_expression ']'  { c_compiler::expr::constant_flag = false; $$ = new c_compiler::parse::designator($3,0); }
  | '.' IDENTIFIER_LEX           { $$ = new c_compiler::parse::designator(0,$2); }
  ;

statement
  : labeled_statement
  | compound_statement
  | expression_statement
  | selection_statement
  | iteration_statement
  | jump_statement
  | asm_statement
  ;

labeled_statement
  : IDENTIFIER_LEX ':' { c_compiler::stmt::label::action($1); } statement
  | CASE_KW { c_compiler::expr::constant_flag = true; } constant_expression ':' { c_compiler::expr::constant_flag = false; c_compiler::stmt::_case($3); } statement
  | DEFAULT_KW ':' { c_compiler::stmt::_default(); } statement
  ;

compound_statement
  : '{' enter_block                 leave_block '}'
  | '{' enter_block block_item_list leave_block '}'
  ;

block_item_list
  : block_item
  | block_item_list block_item
  ;

block_item
  : declaration
  | statement
  ;

expression_statement
  :            ';'
  | expression ';'  { c_compiler::stmt::expr($1); }
  ;

selection_statement
  : if_statement_begin statement { c_compiler::stmt::end_if(); }
  | if_statement_begin statement ELSE_KW { c_compiler::stmt::else_action(); } statement { c_compiler::stmt::end_if(); }
  | switch_statement_begin statement { c_compiler::stmt::end_switch(); }
  ;

if_statement_begin
  : IF_KW '(' expression ')' { c_compiler::stmt::if_expr($3); }
  ;

switch_statement_begin
  : SWITCH_KW '(' expression ')' { c_compiler::stmt::switch_expr($3); }
  ;

iteration_statement
  : while '(' expression ')' { c_compiler::stmt::while_expr($1,$3); } statement  { c_compiler::stmt::end_while(); }
  | do statement while '(' expression ')' ';'  { c_compiler::stmt::end_do($3,$5); }                    
  | for_expr1 for_expr2 for_expr3 statement { c_compiler::stmt::end_for($1); }
  ;

while
  : WHILE_KW
    {
      using namespace c_compiler;
      $$ = new to3ac;
      code.push_back($$);
    }
  ;

do
  : DO_KW { c_compiler::stmt::_do(); }
  ;

for_expr1
  : FOR_KW '('                     ';'  { c_compiler::stmt::for_expr1(0);  $$ = 0; }
  | FOR_KW '('          expression ';'  { c_compiler::stmt::for_expr1($3); $$ = 0; }
  | FOR_KW '(' for_decl declaration     { c_compiler::stmt::for_expr1(0);  $$ = 1; c_compiler::stmt::for_decl = false; }
  ;

for_decl
  : { c_compiler::stmt::for_decl = true; c_compiler::parse::block::enter(); }
  ;

for_expr2
  :            ';'  { c_compiler::stmt::for_expr2(0); }
  | expression ';'  { c_compiler::stmt::for_expr2($1); }
  ;

for_expr3
  :            ')'  { c_compiler::stmt::for_expr3(0); }
  | expression ')'  { c_compiler::stmt::for_expr3($1); }
  ;

jump_statement
  : GOTO_KW IDENTIFIER_LEX ';' { c_compiler::stmt::_goto($2); }
  | CONTINUE_KW ';'            { c_compiler::stmt::_continue(); }
  | BREAK_KW ';'               { c_compiler::stmt::_break(); }
  | RETURN_KW            ';'   { c_compiler::stmt::_return(0); }
  | RETURN_KW expression ';'   { c_compiler::stmt::_return($2); }
  ;

asm_statement
  : ASM_KW '(' string_literal                  ')' ';' { c_compiler::stmt::_asm_::action($3); }
  | ASM_KW '(' string_literal asm_operand_list ')' ';' { c_compiler::stmt::_asm_::action($3,$4); }
  ;

asm_operand_list : ':' asm_operands ':' asm_operands ':' reg_list { $$ = new c_compiler::stmt::_asm_::operand_list($2,$4,$6); }
                 | ':' asm_operands ':' asm_operands              { $$ = new c_compiler::stmt::_asm_::operand_list($2,$4,0); }
                 | ':' asm_operands ':'              ':' reg_list { $$ = new c_compiler::stmt::_asm_::operand_list($2,0,$5); }
                 | ':' asm_operands                               { $$ = new c_compiler::stmt::_asm_::operand_list($2,0,0); }
                 | ':'              ':' asm_operands ':' reg_list { $$ = new c_compiler::stmt::_asm_::operand_list(0,$3,$5); }
                 | ':'              ':' asm_operands              { $$ = new c_compiler::stmt::_asm_::operand_list(0,$3,0); }
                 | ':'              ':'              ':' reg_list { $$ = new c_compiler::stmt::_asm_::operand_list(0,0,$4); }
                 | ':'              ':'              ':'          { $$ = new c_compiler::stmt::_asm_::operand_list(0,0,0); }
                 ;

asm_operands : asm_operand                  { $$ = new c_compiler::stmt::_asm_::operands; $$->push_back($1); }
             | asm_operands ',' asm_operand { $$ = $1; $$->push_back($3); }
             ;

asm_operand : string_literal '(' expression ')' { $$ = new c_compiler::stmt::_asm_::operand($1,$3); }
            ;

reg_list : string_literal              { $$ = new c_compiler::stmt::_asm_::reg_list; $$->push_back($1); }
         | reg_list ',' string_literal { $$ = $1; $$->push_back($3); }
         ;

translation_unit
  : external_declaration
  | translation_unit external_declaration
  ;

external_declaration
  : function_definition  { using namespace c_compiler; destroy_temporary(); parse::is_last_decl = false; }
  | declaration          { using namespace c_compiler; destroy_temporary(); parse::is_last_decl = true; }
  | ';'
  ;

function_definition
  : function_definition_begin compound_statement
    {
      using namespace c_compiler;
      if ( fundef::current ){
        function_definition::action(fundef::current,code,true);
        delete fundef::current;
        fundef::current = 0;
      }
    }
  ;

function_definition_begin
  : declaration_specifiers declarator           { c_compiler::function_definition::begin($1,$2); }
  | declaration_specifiers declarator old_style { c_compiler::function_definition::begin($1,$2); }
  |                        declarator           { c_compiler::function_definition::begin(0,$1); }
  |                        declarator old_style { c_compiler::function_definition::begin(0,$1); }
  ;

old_style
  : begin_declaration_list declaration_list end_declaration_list
  ;

begin_declaration_list
  : {
      using namespace c_compiler::parse;
      parameter::old_style = true;
      parameter::enter2();
      decl_specs::m_stack.push(0);
    }
  ;

end_declaration_list
  : {
      using namespace c_compiler::parse;
      parameter::leave();
      decl_specs::m_stack.pop();
      parameter::old_style = false;
    }
  ;

declaration_list
  :                  declaration
  | declaration_list declaration
  ;

enter_parameter
  : {
      using namespace c_compiler::parse;
      parameter::enter();
      decl_specs::m_stack.push(0);
    }
  ;

leave_parameter
  : {
      using namespace c_compiler::parse;
      parameter::leave();
      decl_specs::m_stack.pop();
    }
  ;

enter_block
  : { c_compiler::parse::block::enter(); }
  ;

leave_block
  : { c_compiler::parse::block::leave(); }
  ;

%%
