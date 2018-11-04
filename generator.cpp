#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"

namespace c_compiler {
  namespace generator {
    void (*generate)(const interface_t*);
    long_double_t* long_double;
    type::id_t sizeof_type = type::UINT;
    type::id_t ptrdiff_type = type::LONG;
    namespace wchar {
      type::id_t id = type::USHORT;
      const c_compiler::type* type = ushort_type::create();
    } // end of namespace wchar
    bool require_align = true;
    int (*close_file)();
    void (*last)(const last_interface_t*);
    void* m_module;
  } // end of namespace generator
} // end of namespace c_compiler

#ifdef _MSC_VER
extern "C" void* __stdcall LoadLibraryA(const char*);
extern "C" void* __stdcall GetProcAddress(void*, const char*);
extern "C" void __stdcall FreeLibrary(void*);
#endif // _MSC_VER

void c_compiler::generator::initialize()
{
  using namespace std;
  string fn = cmdline::generator;
  if ( fn.empty() )
    return;

#ifdef _MSC_VER
  m_module = LoadLibraryA(fn.c_str());
#else // _MSC_VER
  m_module = dlopen(fn.c_str(),RTLD_LAZY);
#endif // _MSC_VER
  if ( !m_module ){
    warning::generator::open(fn);
    return;
  }

#ifdef _MSC_VER
#define dlsym GetProcAddress
#endif // _MSC_VER

  int (*seed)() = (int (*)())dlsym(m_module,"generator_seed");
  if ( !seed )
    warning::generator::seed(fn);
#ifdef _MSC_VER
  int hcc1_seed = _MSC_VER;
  hcc1_seed += 10000000;
#ifdef WIN32
  hcc1_seed += 100000;
#endif // WIN32
#endif // _MSC_VER
#ifdef __GNUC__
  int hcc1_seed = (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__);
  hcc1_seed += 30000000;
#endif // __GNUC__

  int gen_seed = seed ? seed() : hcc1_seed;
  if ( hcc1_seed != gen_seed ){
    warning::generator::seed(fn,make_pair(hcc1_seed,gen_seed));
    return;
  }

  const vector<string>& option = cmdline::g_generator_option;
  if ( !option.empty() ){
    void (*op)(int, const char**, int*) = (void (*)(int, const char**, int*))dlsym(m_module,"generator_option");
    if ( !op ){
      warning::generator::option(fn);
      return;
    }
    vector<const char*> argv;
    argv.push_back(fn.c_str());
    for ( int i = 0 ; i < option.size() ; ++i )
      argv.push_back(option[i].c_str());
    vector<int> error(argv.size());
    (*op)(argv.size(),&argv[0],&error[0]);
    vector<int>::iterator p =
      find_if(error.begin(),error.end(),bind2nd(not_equal_to<int>(),0));
    if ( p != error.end() ){
      int n = distance(error.begin(),p);
      warning::generator::option(option[n],*p,fn);
      return;
    }
  }

  int (*of)(const char*) = (int (*)(const char*))dlsym(m_module,"generator_open_file");
  if (of){
    if (!cmdline::output.empty())
      if ((*of)(cmdline::output.c_str()))
        return;
  }

  void (*spell)(void*) = (void (*)(void*))dlsym(m_module,"generator_spell");
  if (spell) {
    void* magic[] = {
      (void*)&tac_impl::dump,
    };
    (*spell)(&magic[0]);
  }

  generate = (void (*)(const interface_t*))dlsym(m_module,"generator_generate");

  int (*size)(int) = (int (*)(int))dlsym(m_module,"generator_sizeof");
  type_impl::update(size);
  type::id_t (*szof_tp)() = (type::id_t (*)())dlsym(m_module, "generator_sizeof_type");
  if (szof_tp)
    sizeof_type = (*szof_tp)();
  type::id_t (*ptrd_tp)() = (type::id_t (*)())dlsym(m_module, "generator_ptrdiff_type");
  if (ptrd_tp)
    ptrdiff_type = (*ptrd_tp)();

  type::id_t (*wc_tp)() = (type::id_t (*)())dlsym(m_module,"generator_wchar_type");
  if (wc_tp) {
    wchar::id = (*wc_tp)();
    switch (wchar::id) {
    case type::SHORT:  wchar::type = short_type::create(); break;
    case type::USHORT: wchar::type = ushort_type::create(); break;
    case type::INT:    wchar::type = int_type::create(); break;
    case type::UINT:   wchar::type = uint_type::create(); break;
    case type::LONG:   wchar::type = long_type::create(); break;
    default:
      assert(wchar::id == type::ULONG);
      wchar::type = ulong_type::create();
      break;
    }
  }
  
  bool (*ra)() = (bool (*)())dlsym(m_module,"generator_require_align");
  if (ra)
    require_align = (*ra)();

  if ( long_double_t* (*pf)() = (long_double_t* (*)())dlsym(m_module,"generator_long_double") )
    long_double = (*pf)();

  close_file = (int (*)())dlsym(m_module,"generator_close_file");

  last = (void (*)(const last_interface_t*))dlsym(m_module, "generator_last");
}

void c_compiler::generator::terminate()
{
  if ( !cmdline::generator.empty() ){
    if ( generator::close_file )
      generator::close_file();
    if ( error::counter )
#ifdef _MSC_VER
      _unlink(cmdline::output.c_str());
#else // _MSC_VER
      unlink(cmdline::output.c_str());
#endif // _MSC_VER

#ifdef _MSC_VER
    if ( m_module ) FreeLibrary(m_module);
#else // _MSC_VER
    if ( m_module ) dlclose(m_module);
#endif // _MSC_VER
  }
}

