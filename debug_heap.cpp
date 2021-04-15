#include <algorithm>
#include <stdlib.h>

#ifdef DEBUG_HEAP
void* alloced[512*1024];

void save(void* ptr)
{
  int N = sizeof alloced/sizeof alloced[0];
  auto it = std::find(&alloced[0],&alloced[N],nullptr);
  if (it == &alloced[N])
    asm("int3");
  *it = ptr;
}

void discard(void* ptr)
{
  int N = sizeof alloced/sizeof alloced[0];
  auto it = std::find(&alloced[0],&alloced[N],ptr);
  if (it == &alloced[N])
    asm("int3");
  *it = 0;
}

void* operator new(size_t sz)
{
  void* p = malloc(sz);
  save(p);
  return p;
}

void operator delete(void* p, size_t)
{
  discard(p);
  free(p);
}

#endif // DEBUG_HEAP
