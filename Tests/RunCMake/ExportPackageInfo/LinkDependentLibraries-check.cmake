include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/LinkDependentLibraries-build")

file(READ "${out_dir}/dyld.cps" content)
expect_value("${content}" "dyld" "name")
expect_value("${content}" "dylib" "components" "private" "type")
expect_value("${content}" "dylib" "components" "public" "type")

expect_array(
  "${content}" 1
  "components" "public" "configurations" "generic" "dyld_requires")
expect_value(
  "${content}" ":private"
  "components" "public" "configurations" "generic" "dyld_requires" 0)
