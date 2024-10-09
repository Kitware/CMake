#include "c_std.h"
#if defined(C_STD) && C_STD <= C_STD_17 &&                                    \
  !(C_STD == C_STD_17 && defined(__clang_major__) && __clang_major__ < 14)
#  error "c_std_23 not honored"
#endif
