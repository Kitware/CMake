include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/FileSetHeaders-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

file(READ "${out_dir}/foo.cps" content)

string(JSON component GET "${content}" "components" "foo")

expect_array("${component}" 1 "includes")
expect_value("${component}" "@prefix@/no_genex" "includes" 0)

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
