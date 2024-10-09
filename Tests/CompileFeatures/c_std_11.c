#include "c_std.h"
#if defined(C_STD) && C_STD <= C_STD_99 &&                                    \
  !(defined(__SUNPRO_C) && __SUNPRO_C < 0x5140 && C_STD == C_STD_99)
#  error "c_std_11 not honored"
#endif
