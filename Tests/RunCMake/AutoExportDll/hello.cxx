#include "hello.h"

#include <stdio.h>
int Hello::Data = 0;
void Hello::real()
{
  return;
}
void hello()
{
  printf("hello");
}
void Hello::operator delete[](void*) {};
void Hello::operator delete(void*) {};

#ifdef HELLO_VFTABLE
HelloVFTable::HelloVFTable()
{
}
HelloVFTable::~HelloVFTable()
{
}
#endif

#ifndef __SUNPRO_CC
// C++ operators incorrectly declared extern "C" should *not* be exported.
extern "C" {
bool operator==(Hello const&, Hello const&)
{
  return false;
}
bool operator!=(Hello const&, Hello const&)
{
  return false;
}
bool operator<(Hello const&, Hello const&)
{
  return false;
}
bool operator<=(Hello const&, Hello const&)
{
  return false;
}
bool operator>(Hello const&, Hello const&)
{
  return false;
}
bool operator>=(Hello const&, Hello const&)
{
  return false;
}
}
#endif
