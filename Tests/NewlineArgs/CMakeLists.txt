# a simple CXX only test case
project (newlineargs CXX)

add_definitions("-DTEST_FLAG_1
-DTEST_FLAG_2")

include_directories("${newlineargs_BINARY_DIR} 
${newlineargs_SOURCE_DIR}")

configure_file("${newlineargs_SOURCE_DIR}/libcxx2.h.in" 
  "${newlineargs_BINARY_DIR}/libcxx2.h")

add_library(testcxx1 libcxx1.cxx)
add_executable (cxxonly cxxonly.cxx)
target_link_libraries(cxxonly testcxx1)
