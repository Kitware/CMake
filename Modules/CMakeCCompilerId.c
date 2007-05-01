#ifdef __cplusplus
# error "A C++ compiler has been selected for C."
#endif

#ifdef __CLASSIC_C__
# define const
#endif

static char const info_compiler[] = "INFO:compiler["
#if defined(__INTEL_COMPILER) || defined(__ICC)
"Intel"
#elif defined(__BORLANDC__)
"Borland"
#elif defined(__WATCOMC__)
"Watcom"
#elif defined(__SUNPRO_C)
"SunPro"
#elif defined(__HP_cc)
"HP"
#elif defined(__DECC)
"Compaq"
#elif defined(__IBMC__)
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
