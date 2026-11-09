include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(BISON REQUIRED)

bison_target(C parser.y parser.c)
check_var(BISON_C_DEFINED TRUE)
check_var(BISON_C_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/parser.y")
check_var(BISON_C_OUTPUT_SOURCE "parser.c")
check_var(BISON_C_OUTPUT_HEADER "parser.h")
check_var(BISON_C_OUTPUTS "parser.c;parser.h")
check_var(BISON_C_OPTIONS "-d")
check_var(BISON_C_COMPILE_FLAGS "-d")

# The default header name replaces 'c' with 'h' in the extension.
bison_target(CXX parser.y parser.cpp)
check_var(BISON_CXX_OUTPUT_HEADER "parser.hpp")
bison_target(CC parser.y parser.cc)
check_var(BISON_CC_OUTPUT_HEADER "parser.hh")
bison_target(CXX2 parser.y parser.cxx)
check_var(BISON_CXX2_OUTPUT_HEADER "parser.hxx")

bison_target(Subdir parser.y sub/parser.c)
check_var(BISON_Subdir_OUTPUT_SOURCE "sub/parser.c")
check_var(BISON_Subdir_OUTPUT_HEADER "sub/parser.h")
check_var(BISON_Subdir_OUTPUTS "sub/parser.c;sub/parser.h")

bison_target(Absolute "${CMAKE_CURRENT_SOURCE_DIR}/parser.y"
  "${CMAKE_CURRENT_BINARY_DIR}/absolute.c")
check_var(BISON_Absolute_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/parser.y")
check_var(BISON_Absolute_OUTPUT_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/absolute.c")
check_var(BISON_Absolute_OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/absolute.h")
