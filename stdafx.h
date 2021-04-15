#ifndef _STDAFX_H_
#define _STDAFX_H_

#include <vector>
#include <string>
#include <map>
#include <stack>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <functional>
#include <algorithm>
#include <memory>
#include <list>
#include <numeric>
#include <typeinfo>
#include <limits.h>
#include <cstdio>
#include <cassert>
#include <set>
#include <limits>
#include <cmath>
#include <cstring>
#include <cerrno>
#include <stdint.h>
#include <fcntl.h>

#include <sys/stat.h>

#ifndef _MSC_VER
#include <dlfcn.h>
#include <unistd.h>
#define __int64 long long int
#endif // _MSC_VER

#ifdef DEBUG_HEAP
void* operator new(size_t sz);
void operator delete(void* p, size_t);
#endif // DEBUG_HEAP

#endif // _STDAFX_H_
