#include <stdio.h>
#include "CMakeCompilerABI.h"

int main(int argc, char* argv[]) {
  int require = 0;
  require += info_sizeof_dptr[argc];
  require += info_byte_order_big_endian[argc];
  require += info_byte_order_little_endian[argc];
#if defined(ABI_ID)
  require += info_abi[argc];
#endif
  printf("%d\n", require);
  return 0;
}
