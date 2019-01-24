%{
#ifdef _MSC_VER
#define YY_NO_UNISTD_H
#define isatty _isatty
#define fileno _fileno
extern "C" int isatty(int);
#endif // _MSC_VER

#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "c_y.h"

namespace c_compiler { namespace parse {

void line_control(bool ms)
{
  using namespace std;
  istringstream is(yytext);
  char sharpe;
  is >> sharpe;
  if (ms) {
    string line;
    is >> line;
    assert(line == "line");
  }
  is >> position.m_lineno;
  --position.m_lineno;
  if (ms) {
    char dq = '\0';
    is >> dq;
    if (dq == '"') {
      string fn;
      char c;
      while (is.get(c)) {
        if (c == dq)
          break;
        if (c == '\\') {
          is.get(c);
          assert(c == '\\');
        }
        fn += c;
      }
      position.m_name = fn;
    }
  }
  else {
    std::string fn;
    is >> fn;
    if ( !fn.empty() )
      position.m_name = fn.substr(1,fn.size()-2);
  }
}

} } // end of namespace parse and c_compiler

%}

hex-quad  {hexadecimal-digit}{4}
universal-character-name  \\u{hex-quad}|\\U{hex-quad}{hex-quad}

identifier  {nondigit}({nondigit}|{digit})*
nondigit  {universal-character-name}|[_a-zA-Z]
digit  [0-9]

floating-constant  {decimal-floating-constant}|{hexadecimal-floating-constant}
decimal-floating-constant  {fractional-constant}{exponent-part}?{floating-suffix}?|{digit-sequence}{exponent-part}{floating-suffix}?
hexadecimal-floating-constant  0[xX]({hexadecimal-fractional-constant}|{hexadecimal-digit-sequence}){binary-exponent-part}{floating-suffix}?
fractional-constant  {digit-sequence}?\.{digit-sequence}|{digit-sequence}\.
exponent-part  [eE]{sign}?{digit-sequence}
sign  [+-]
digit-sequence  {digit}+
hexadecimal-fractional-constant  ({hexadecimal-digit-sequence}?\.{hexadecimal-digit-sequence})|({hexadecimal-digit-sequence}\.)
binary-exponent-part  [pP]{sign}?{digit-sequence}
hexadecimal-digit-sequence  {hexadecimal-digit}+
hexadecimal-digit  [0-9a-fA-F]
floating-suffix  [flFL]

integer-constant  {decimal-constant}{integer-suffix}?|{octal-constant}{integer-suffix}?|{hexadecimal-constant}{integer-suffix}?
decimal-constant  {nonzero-digit}{digit}*
octal-constant  0{octal-digit}*
hexadecimal-constant  0[xX]{hexadecimal-digit}+
nonzero-digit  [1-9]
octal-digit  [0-7]
integer-suffix  {unsigned-suffix}{long-suffix}?|{long-suffix}{unsigned-suffix}?|{unsigned-suffix}{long-long-suffix}?|{long-long-suffix}{unsigned-suffix}?
unsigned-suffix  u|U
long-suffix  l|L
long-long-suffix  ll|LL

character-constant  L?'{c-char-sequence}'
c-char-sequence  {c-char}+
c-char  [^'\\\n\r]|{escape-sequence}|{universal-character-name}
escape-sequence  {simple-escape-sequence}|{octal-escape-sequence}|{hexadecimal-escape-sequence}
simple-escape-sequence  \\'|\\\"|\\\?|\\\\|\\a|\\b|\\f|\\n|\\r|\\t|\\v
octal-escape-sequence  \\{octal-digit}{1,3}
hexadecimal-escape-sequence  \\x{hexadecimal-digit}+

string-literal  L?\"{s-char-sequence}?\"
s-char-sequence  {s-char}+
s-char  [^\"\\\n\r]|{escape-sequence}|{universal-character-name}

%%

__builtin_va_arg    { return BUILTIN_VA_ARG; }
__builtin_va_end    { return BUILTIN_VA_END; }
__builtin_va_start  { return BUILTIN_VA_START; }
asm                 { return ASM_KW; }
auto                { return AUTO_KW; }
break               { return BREAK_KW; }
case                { return CASE_KW; }
char                { return CHAR_KW; }
const               { return CONST_KW; }
continue            { return CONTINUE_KW; }
default             { return DEFAULT_KW; }
double              { return DOUBLE_KW; }
do                  { return DO_KW; }
else                { return ELSE_KW; }
enum                { c_compiler::parse::decl_specs::m_curr.push_back(ENUM_KW); return c_compiler::parse::guess(ENUM_KW); }
extern              { return EXTERN_KW; }
float               { return FLOAT_KW; }
for                 { return FOR_KW; }
goto                { return GOTO_KW; }
if                  { return IF_KW; }
inline              { return INLINE_KW; }
int                 { return INT_KW; }
long                { return LONG_KW; }
register            { return REGISTER_KW; }
restrict            { return RESTRICT_KW; }
return              { return RETURN_KW; }
short               { return SHORT_KW; }
signed              { return SIGNED_KW; }
sizeof              { c_compiler_lval.m_ival = c_compiler::code.size(); return SIZEOF_KW; }
static              { return STATIC_KW; }
struct              { return c_compiler::parse::guess(STRUCT_KW); }
switch              { return SWITCH_KW; }
typedef             { return TYPEDEF_KW; }
union               { return c_compiler::parse::guess(UNION_KW); }
unsigned            { return UNSIGNED_KW; }
void                { return VOID_KW; }
volatile            { return VOLATILE_KW; }
while               { c_compiler_lval.m_ival = c_compiler::code.size(); return WHILE_KW; }


"!="      { return NOTEQ_MK; }
"%="      { return MOD_ASSIGN_MK; }
"&&"      { c_compiler_lval.m_ival = c_compiler::code.size(); return ANDAND_MK; }
"&="      { return AND_ASSIGN_MK; }
"*="      { return MUL_ASSIGN_MK; }
"++"      { return PLUSPLUS_MK; }
"+="      { return ADD_ASSIGN_MK; }
"--"      { return MINUSMINUS_MK; }
"-="      { return SUB_ASSIGN_MK; }
"->"      { return ARROW_MK; }
"..."     { return DOTS_MK; } 
"/="      { return DIV_ASSIGN_MK; }
"<<"      { return LSH_MK; }
"<<="     { return LSH_ASSIGN_MK; }
"<="      { return LESSEQ_MK; }
"=="      { return EQUAL_MK; }
">="      { return GREATEREQ_MK; }
">>"      { return RSH_MK; }
">>="     { return RSH_ASSIGN_MK; }
"^="      { return XOR_ASSIGN_MK; }
"|="      { return OR_ASSIGN_MK; }
"||"      { c_compiler_lval.m_ival = c_compiler::code.size(); return OROR_MK; }
"<:"      { return '['; }
":>"      { return ']'; }
"<%"      { return '{'; }
"%>"      { return '}'; }

#                                  {}
#[ \t]*pragma[ \t]*.*              {}
#[ \t]*{digit}+([ \t]+\"[^\"]+\")?([ \t]*{digit})* { c_compiler::parse::line_control(false); }
#[ \t]*line[ \t]*{digit}+([ \t]+\"[^\"]+\")?       { c_compiler::parse::line_control(true); }

{identifier}               { return c_compiler::parse::identifier::judge(c_compiler::ucn::conv(yytext)); }
{integer-constant}         { c_compiler_lval.m_usr = c_compiler::integer::create(yytext); return INTEGER_CONSTANT_LEX; }
{character-constant}       { c_compiler_lval.m_usr = c_compiler::character::create(yytext); return CHARACTER_CONSTANT_LEX; }
{floating-constant}        { c_compiler_lval.m_usr = c_compiler::floating::create(yytext); return FLOATING_CONSTANT_LEX; }
{string-literal}           { using namespace c_compiler; c_compiler_lval.m_var = literal::stringa(ucn::conv(yytext)); return STRING_LITERAL_LEX; }
[ \t]                      {}
\n|\r\n                    { ++c_compiler::parse::position.m_lineno; }
\?                         { c_compiler_lval.m_ival = c_compiler::code.size(); return '?'; }
\:                         { c_compiler_lval.m_ival = c_compiler::code.size(); return ':'; }
\(                         {
                             if ( c_compiler::parse::parameter::depth )
                               c_compiler::parse::maybe_absdecl = true;
                             return '(';
                           }
\)                         {
                             if ( c_compiler::parse::parameter::depth )
                               c_compiler::parse::maybe_absdecl = false;
                             return ')';
                           }
\[                         {
                             ++c_compiler::parse::index_depth;
                             return '[';
                           }
\]                         {
                             --c_compiler::parse::index_depth;
                             return ']';
                           }
.                          { return yytext[0]; }

%%

#ifdef _DEBUG
void c_compiler::parse::delete_buffer()
{
  yy_delete_buffer(YY_CURRENT_BUFFER);
}
#endif // _DEBUG

c_compiler::file_t c_compiler::parse::position;

bool c_compiler::parse::is_last_decl;

int yywrap(){ return 1; }

void c_compiler::parse::unputer(std::string s)
{
  int size = s.length();
  for ( int i = 0 ; i != size ; ++i ) {
    char c = s[size-i-1];
    if (c == '[')
      --index_depth;
    if (c == ']')
      ++index_depth;
    unput(c);
  }
}
