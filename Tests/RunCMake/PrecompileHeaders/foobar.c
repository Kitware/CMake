#include "bar.h"
#include "foo.h"
#include "foo2.h"

int main(void)
{
  int zeroSize = 0;

#ifdef HAVE_PCH_SUPPORT
  zeroSize = (int)strlen("");
#endif

  return foo() + foo2() + bar() + zeroSize;
}
