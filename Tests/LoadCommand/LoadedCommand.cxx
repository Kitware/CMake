#include "LoadedCommand.h"

int main ()
{
#ifdef CMAKE_IS_FUN
  return SIZEOF_CHAR-1;
#else  
  return SIZEOF_SHORT;
#endif
}
