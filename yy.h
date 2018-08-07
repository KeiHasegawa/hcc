#ifndef _YY_H_
#define _YY_H_

extern "C" int c_compiler_wrap();
extern int c_compiler_lex();
extern void c_compiler_error(const char*);
extern char* c_compiler_text;
extern FILE* c_compiler_in;
extern int c_compiler_parse();

#endif // _YY_H_
