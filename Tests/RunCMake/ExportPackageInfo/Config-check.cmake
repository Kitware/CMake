include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/Config-build")

file(READ "${out_dir}/foo.cps" content)

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  foreach(config FooConfig BarConfig)
    expect_object("${content}" "components" "foo" "configurations" ${config})
  endforeach()
else()
  set(config TestConfig)
  expect_object("${content}" "components" "foo" "configurations" ${config})
endif()
