
#ifndef FOO_LIBRARY
#error Expected FOO_LIBRARY
#endif

#ifndef BAR_LIBRARY
#error Expected BAR_LIBRARY
#endif

#ifdef BANG_LIBRARY
#error Unexpected BANG_LIBRARY
#endif

#include "bar.h"

int main(void)
{
  return foo() + bar();
}
