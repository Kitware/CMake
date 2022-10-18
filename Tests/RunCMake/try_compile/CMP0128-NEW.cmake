cmake_policy(SET CMP0128 NEW)
set(check_cxx_std "
#if __cplusplus > 199711L && __cplusplus <= 201103L
#  error Compiler is incorrectly in C++11 mode.
#endif
")
include(CMP0128-common.cmake)
