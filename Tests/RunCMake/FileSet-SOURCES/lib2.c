
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

#if defined(EXPECT_CXX)
#  if !defined(__cplusplus)
#    error "Expected this source to be compiled as C++"
#  endif
#  if !defined(EXPECTED_LANGUAGE)
#    error "COMPILE_LANGUAGE did not use the file set language"
#  endif
#endif

#ifdef __cplusplus
extern "C"
#endif
  void
  f2(void)
{
}
