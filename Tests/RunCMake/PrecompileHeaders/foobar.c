#include "bar.h"
#include "foo.h"
#include "foo2.h"

int main()
{
  return foo() + foo2() + bar();
}
