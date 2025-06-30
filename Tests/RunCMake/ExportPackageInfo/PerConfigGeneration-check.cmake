include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/PerConfigGeneration-build")

file(READ "${out_dir}/foo.cps" content)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    expect_object("${content}" "components" "foo" "configurations" ${config})
  endforeach()
else()
  include(${RunCMake_TEST_BINARY_DIR}/build_type.cmake)
  expect_object("${content}" "components" "foo" "configurations" ${config})
endif()
