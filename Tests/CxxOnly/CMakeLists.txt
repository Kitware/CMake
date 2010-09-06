# a simple CXX only test case
project (CxxOnly CXX)

set(CMAKE_DEBUG_POSTFIX "_test_debug_postfix")
if(MSVC)
  set(EXTRA_SRCS test.CPP)
endif()
add_library(testcxx1 STATIC libcxx1.cxx ${EXTRA_SRCS})
add_library(testcxx2 SHARED libcxx2.cxx)
add_executable (CxxOnly cxxonly.cxx)
target_link_libraries(CxxOnly testcxx1 testcxx2)
