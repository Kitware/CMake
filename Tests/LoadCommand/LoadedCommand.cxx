#include "LoadedCommand.h"
#include <stdio.h>

int main ()
{
#ifdef HAVE_VSBLABLA
  printf("Should not be able to find vsblabla\n");
  return 1;
#endif

#if !defined( HAVE_PRINTF )
  printf("Should be able to find printf\n");
  return 1;
#endif

#if !defined( ADDED_DEFINITION )
  printf("Should have ADDED_DEFINITION defined\n");
  return 1;
#endif

#ifdef CMAKE_IS_FUN
  return SIZEOF_CHAR-1;
#else  
  return SIZEOF_SHORT;
#endif
}
