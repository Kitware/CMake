#include <cstdlib>

#include <c2_lib.h>
#include <shared_lib.h>
#include <static_lib.h>

int main()
{
  static_hello();
  shared_hello();
  c2_hello();
  return EXIT_SUCCESS;
}
