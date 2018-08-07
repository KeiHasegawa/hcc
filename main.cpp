#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"
#include "yy.h"

int main(int argc, char** argv)
{
  using namespace std;
  using namespace c_compiler;
  cmdline::setup(argc,argv);
  if ( !cmdline::input.empty() ){
#ifdef _MSC_VER
	fopen_s(&c_compiler_in, cmdline::input.c_str(), "r");
#else // _MSC_VER
    c_compiler_in = fopen(cmdline::input.c_str(),"r");
#endif // _MSC_VER
    if ( !c_compiler_in ){
      error::cmdline::open(cmdline::input);
      exit(1);
    }
    parse::position = file_t(cmdline::input,1);
  }
  {
#ifdef USE_PROFILE
    simple_profiler::sentry mark("main");
#endif // USE_PROFILE
    generator::initialize();
    c_compiler_parse();
  }
#ifdef _DEBUG
  parse::delete_buffer();
#endif // _DEBUG
  if (!error::counter) {
	  if (generator::wrap) {
		  transform(funcs.begin(), funcs.end(), back_inserter(scope::root.m_children), get_pm);
		  generator::wrap_interface_t tmp = {
			  &scope::root,
			  &funcs
		  };
		  generator::wrap(&tmp);
	  }
  }
  generator::terminate();
#ifdef USE_PROFILE
  simple_profiler::sentry::output();
#endif // USE_PROFILE
  return error::counter;
}
