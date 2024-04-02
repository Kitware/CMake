#include "cxx_std.h"
#if defined(CXX_STD) && CXX_STD < CXX_STD_11 &&                               \
  !(CXX_STD == CXX_STD_98 &&                                                  \
    (defined(__IBMCPP__) && defined(_AIX) && __IBMCPP__ == 1610))
#  error "cxx_std_11 not honored"
#endif
