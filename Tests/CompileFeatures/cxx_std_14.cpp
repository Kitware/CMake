#include "cxx_std.h"
#if defined(CXX_STD) && CXX_STD <= CXX_STD_11 &&                              \
  !(CXX_STD == CXX_STD_11 &&                                                  \
    ((defined(__GNUC__) && defined(__GNUC_MINOR__) && __GNUC__ == 4 &&        \
      __GNUC_MINOR__ <= 8 && !defined(__clang__) &&                           \
      !defined(__INTEL_COMPILER) && !defined(__PGI)) ||                       \
     (defined(__SUNPRO_CC) && __SUNPRO_CC < 0x5150)))
#  error "cxx_std_14 not honored"
#endif
