# This test will verify if CheckSymbolExists only report symbols available
# for linking that really are. You can find some documentation on this in
# bug 11333 where we found out that gcc would optimize out the actual
# reference to the symbol, so symbols that are in fact _not_ available in the
# given libraries (but seen in header) were reported as present.
#
# If you change this test do not forget to change the CheckCXXSymbolExists
# test, too.

cmake_minimum_required(VERSION 2.8)

set(_test_dir "${CMAKE_CURRENT_BINARY_DIR}/symbol_c")
set(_test_build_dir "${CMAKE_CURRENT_BINARY_DIR}/symbol_build")
file(MAKE_DIRECTORY "${_test_dir}" "${_test_build_dir}")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt.in"
  "${_test_dir}/CMakeLists.txt" @ONLY)
ADD_TEST(CheckSymbolExists
  ${CMAKE_CMAKE_COMMAND}
    -G "${CMAKE_GENERATOR}"
    "${_test_dir}"
  )
set_tests_properties(CheckSymbolExists PROPERTIES
  WORKING_DIRECTORY "${_test_build_dir}")
