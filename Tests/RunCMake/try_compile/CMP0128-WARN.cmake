
set(check_cxx_std "
#if __cplusplus <= 199711L || __cplusplus > 201103L
#  error Compiler is incorrectly not in C++11 mode.
#endif
")
include(CMP0128-common.cmake)
