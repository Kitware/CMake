# a simple CXX only test case
project (cxxonly CXX)

set(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")
add_library(testcxx1 STATIC libcxx1.cxx)
add_library(testcxx2 SHARED libcxx2.cxx)
add_executable (cxxonly cxxonly.cxx)
target_link_libraries(cxxonly testcxx1 testcxx2)
