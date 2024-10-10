#include "cxx_std.h"
#if defined(CXX_STD) && CXX_STD < CXX_STD_11
#  error "hip_std_11 not honored"
#endif
