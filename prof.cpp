#include "stdafx.h"
#include "c_core.h"
#include "parse.h"
#include "misc.h"

#ifdef USE_PROFILE
c_compiler::simple_profiler::sentry::sentry(std::string label) : m_id(counter)
{
  all.resize(++counter);
  info& b = all.back();
  b.m_label = label;
  QueryPerformanceCounter(&b.m_start);
}

int c_compiler::simple_profiler::sentry::counter;

c_compiler::simple_profiler::sentry::~sentry()
{
  info& b = all[m_id];
  QueryPerformanceCounter(&b.m_end);
}

std::vector<c_compiler::simple_profiler::info> c_compiler::simple_profiler::sentry::all;

namespace c_compiler { namespace simple_profiler {
  int subr1(const info&);
  std::map<std::string, __int64> table;
  int subr2(const std::pair<std::string, __int64>&);
} } // end of namespace simple_profiler and c_compiler

void c_compiler::simple_profiler::sentry::output()
{
  using namespace std;
  const vector<info>& v = sentry::all;
  for_each(v.begin(),v.end(),subr1);
  cout << '\n';
  for_each(table.begin(),table.end(),subr2);
}

int c_compiler::simple_profiler::subr1(const info& i)
{
  using namespace std;
  __int64 v = i.m_end.QuadPart - i.m_start.QuadPart;
  string k = i.m_label;
  cout << k << " : ";
  printf("%lld\n", v);
  table[k] += v;
  return 0;
}

int c_compiler::simple_profiler::subr2(const std::pair<std::string, __int64>& i)
{
  using namespace std;
  string k = i.first;
  __int64 v = i.second;
  cout << k << " : ";
  printf("%lld\n", v);
  return 0;
}
#endif // USE_PROFILE
