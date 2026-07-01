include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/FileSetHeaders-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)

string(JSON component GET "${content}" "components" "foo")

expect_array("${component}" 1 "includes")
expect_value("${component}" "@prefix@/no_genex" "includes" 0)

expect_array("${component}" 2 "file_sets")

expect_value("${component}" "includes" "file_sets" 0 "type")
expect_value("${component}" "@prefix@/no_genex" "file_sets" 0 "root")
expect_array("${component}" 2 "file_sets" 0 "files")
expect_value("${component}" "header1.h" "file_sets" 0 "files" 0)
expect_value("${component}" "header2.h" "file_sets" 0 "files" 1)
expect_value("${component}" "no_genex"
             "file_sets" 0 "extensions" "cmake" "name@v1")

expect_value("${component}" "includes" "file_sets" 1 "type")
expect_value("${component}" "@prefix@/no_genex" "file_sets" 1 "root")
expect_array("${component}" 2 "file_sets" 1 "files")
expect_value("${component}" "header1.h" "file_sets" 1 "files" 0)
expect_value("${component}" "header2.h" "file_sets" 1 "files" 1)
expect_value("${component}" "no_genex_dup"
             "file_sets" 1 "extensions" "cmake" "name@v1")

file(GLOB configs "${out_dir}/foo@*.cps")
list(LENGTH configs configs_len)

if(NOT configs_len)
  set(RunCMake_TEST_FAILED
    "No configuration-specific CPS files were generated" PARENT_SCOPE)
  return()
endif()

foreach(config ${configs})
  file(READ "${config}" content)

  string(JSON component GET "${content}" "components" "foo")

  expect_array("${component}" 1 "includes")
  expect_value("${component}" "@prefix@/genex" "includes" 0)
endforeach()
