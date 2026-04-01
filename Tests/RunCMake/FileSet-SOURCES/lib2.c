
#if defined(CONSUMER)
#  if defined(LIB1_A)
#    error "LIB1_A defined"
#  endif
#  if !defined(INTERFACE_LIB1_A)
#    error "INTERFACE_LIB1_A not defined"
#  endif

#  include "h2.h"
#else
#  if !defined(LIB1_A)
#    error "LIB1_A not defined"
#  endif
#  if defined(INTERFACE_LIB1_A)
#    error "INTERFACE_LIB1_A defined"
#  endif

#  include "h1.h"
#endif

void f2(void)
{
}
