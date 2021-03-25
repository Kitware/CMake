#include "cxx_pch.h"
#ifndef CXX_PCH
#  error "CXX PCH not included in CXX source."
#endif
extern "C" int no_pch(void);
int main()
{
  return no_pch();
}
