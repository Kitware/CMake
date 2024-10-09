#include "c_std.h"
#if defined(C_STD) && C_STD < C_STD_99
#  error "c_std_99 not honored"
#endif
