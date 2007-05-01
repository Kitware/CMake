/* This source file must have a .cpp extension so that all C++ compilers
   recognize the extension without flags.  Borland does not know .cxx for
   example.  */
#ifndef __cplusplus
# error "A C compiler has been selected for C++."
#endif

static char const info_compiler[] = "INFO:compiler["
#if defined(__COMO__)
"Comeau"
#elif defined(__INTEL_COMPILER) || defined(__ICC)
"Intel"
#elif defined(__BORLANDC__)
"Borland"
#elif defined(__WATCOMC__)
"Watcom"
#elif defined(__SUNPRO_CC)
"SunPro"
#elif defined(__HP_aCC)
"HP"
#elif defined(__DECCXX)
"Compaq"
#elif defined(__IBMCPP__)
"VisualAge"
#elif defined(__GNUC__)
"GNU"
#elif defined(_MSC_VER)
"MSVC"
#elif defined(_COMPILER_VERSION)
"MIPSpro"

/* This compiler is either not known or is too old to define an
   identification macro.  Try to identify the platform and guess that
   it is the native compiler.  */
#elif defined(__sgi)
"MIPSpro"
#elif defined(__hpux) || defined(__hpua)
"HP"

#else /* unknown compiler */
""
#endif
"]";

/* Include the platform identification source.  */
#include "CMakePlatformId.h"

/* Make sure the information strings are referenced.  */
int main()
{
  return ((int)&info_compiler) + ((int)&info_platform);
}
