#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"

namespace c_compiler {  namespace cmdline {
  struct table_t : std::map<std::string,int (*)(char**)> {
    table_t();
  } table;
  bool no_generator;
} } // end of namespace cmdline and c_compiler

std::string c_compiler::cmdline::prog;

std::string c_compiler::cmdline::input;

std::string c_compiler::cmdline::output;

void c_compiler::cmdline::setup(int argc, char** argv)
{
  using namespace std;
#ifdef _MSC_VER
  int separator = '\\';
#else // _MSC_VER
  int separator = '/';
#endif // _MSC_VER
  prog = *argv;
  while ( *++argv ){
    if ( **argv == '-' ){
      table_t::const_iterator p = table.find(*argv);
      if ( p != table.end() )
        argv += (p->second)(argv);
      else
        warning::cmdline::option(*argv);
    }
    else {
      if ( input.empty() ){
        input = *argv;
        if (output.empty()) {
          output = input;
          string::size_type pos = output.find_last_of(separator);
          if (pos != string::npos)
            output.erase(0, pos + 1);
          pos = output.find_last_of('.');
          if (pos != string::npos)
            output.erase(pos);
          output += ".s";
        }
      }
      else
        warning::cmdline::input(*argv);
    }
  }

  if ( generator.empty() && !no_generator ){
#ifdef _MSC_VER
    char* p;
    size_t n;
    _dupenv_s(&p,&n,"CC1GENERATOR");
#else // _MSC_VER
    char* p = getenv("CC1GENERATOR");
#endif // _MSC_VER
    if ( p )
      generator = p;
    else
      error::cmdline::generator();
  }
}

namespace c_compiler {
  namespace cmdline {
    int no_generator_option(char**);
    int output_medium_option(char**);
    int simple_medium_option(char**);
    int generator_option(char**);
    int generator_option_option(char**);
    int o_option(char**);
    int lang_option(char**);
    int version_option(char**);
    int optimize_option(char**);
    int nobb_optimize_option(char**);
    int nodag_optimize_option(char**);
    int output_optinfo_option(char**);
    int help_option(char**);
  }
}

c_compiler::cmdline::table_t::table_t()
{
  (*this)["--no-generator"] = no_generator_option;
  (*this)["--output-medium"] = output_medium_option;
  (*this)["--simple-medium"] = simple_medium_option;
  (*this)["--generator"] = generator_option;
  (*this)["--generator-option"] = generator_option_option;
  (*this)["-o"] = o_option;
  (*this)["--lang"] = lang_option;
  (*this)["--version"] = version_option;
  (*this)["--optimize"] = optimize_option;
  (*this)["--no-basic-block-optimize"] = nobb_optimize_option;
  (*this)["--no-dag-optimize"] = nodag_optimize_option;
  (*this)["--output-optinfo"] = output_optinfo_option;
  (*this)["-h"] = help_option;
}

int c_compiler::cmdline::no_generator_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : not specify generator";
    return 0;
  }

  no_generator = true;
  return 0;
}

bool c_compiler::cmdline::output_medium;

int c_compiler::cmdline::output_medium_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : dump 3 address code and symbol table";
    return 0;
  }

  output_medium = true;
  return 0;
}

bool c_compiler::cmdline::simple_medium;

int c_compiler::cmdline::simple_medium_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : name compiler medium variables simply when dump";
    return 0;
  }

  simple_medium = true;
  return 0;
}

bool c_compiler::cmdline::output_optinfo = false;

int c_compiler::cmdline::output_optinfo_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : output optimization infomation";
    return 0;
  }

  output_optinfo = true;
  return 0;
}

std::string c_compiler::cmdline::generator;

int c_compiler::cmdline::generator_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " GENERATOR : specify generator";
    return 0;
  }

  if ( *++argv && **argv != '-' ){
    generator = *argv;
    return 1;
  }
  else {
    warning::cmdline::generator_option();
    return 0;
  }
}

std::vector<std::string> c_compiler::cmdline::g_generator_option;

int c_compiler::cmdline::generator_option_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " ( GENERATOR-OPTION ) : pass option to generator";
    return 0;
  }

  string left = "(";
  if ( !*++argv || *argv != left ){
    warning::cmdline::generator_option_option(left);
    return 0;
  }

  int n = 1;
  string right = ")";
  for ( ; *++argv ; ++n ){
    if ( right == *argv )
      return ++n;
    g_generator_option.push_back(*argv);
  }

  warning::cmdline::generator_option_option(right);
  return n;
}

int c_compiler::cmdline::o_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " OUTPUTFILE : specify output file";
    return 0;
  }

  if ( *++argv && **argv != '-' ){
    output = *argv;
    return 1;
  }
  else {
    warning::cmdline::o_option();
    return 0;
  }
}

namespace c_compiler { namespace cmdline { namespace lang_impl {
  struct table : std::map<std::string, error::LANG> {
    table();
  } m_table;
} } } // end of namespace lang_impl, cmdline and c_compiler

int c_compiler::cmdline::lang_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " LANG : specify error message language";
    return 0;
  }

  if ( *++argv && **argv != '-' ){
    map<string, error::LANG>::const_iterator p
      = lang_impl::m_table.find(*argv);
    if ( p != lang_impl::m_table.end() )
      error::lang = p->second;
    else
      warning::cmdline::lang_option(*argv);
    return 1;
  }
  else {
    warning::cmdline::lang_option();
    return 0;
  }
}

c_compiler::cmdline::lang_impl::table::table()
{
  (*this)["jpn"] = error::jpn;
  (*this)["eng"] = error::other;
}

int c_compiler::cmdline::version_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : output version";
    return 0;
  }
  cerr << prog << ": version " << "1.3" << '\n';
  return 0;
}

int c_compiler::cmdline::optimize_level = 1;

int c_compiler::cmdline::optimize_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " LEVEL : specify optimize level";
    return 0;
  }

  if ( *++argv && '0' <= **argv && **argv <= '1' ){
    optimize_level = **argv - '0';
    return 1;
  }
  else {
    warning::cmdline::optimize_option();
    return 0;
  }
}

bool c_compiler::cmdline::bb_optimize = true;

int c_compiler::cmdline::nobb_optimize_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : not apply basic block optimization";
    return 0;
  }

  bb_optimize = false;
  return 0;
}

bool c_compiler::cmdline::dag_optimize = true;

int c_compiler::cmdline::nodag_optimize_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : not apply dag optimization";
    return 0;
  }

  dag_optimize = false;
  return 0;
}

namespace c_compiler {
  namespace cmdline {
    using namespace std;    
    void help(const pair<string, int (*)(char**)>& p)
    {
      cerr << '\t' << p.first;
      p.second(0);
      cerr << '\n';
    }
  } // end of namespace cmdline
} // end of namespace c_compiler

int c_compiler::cmdline::help_option(char** argv)
{
  using namespace std;
  if ( !argv ){
    cerr << " : output this message";
    return 0;
  }

  for_each(table.begin(),table.end(),help);
  return 0;
}
