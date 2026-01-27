include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/Config-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

macro(check_config CONFIG)
  string(TOLOWER "${CONFIG}" CONFIG_LOWER)
  file(READ "${out_dir}/foo@${CONFIG_LOWER}.cps" content)
  expect_value("${content}" "${CONFIG}" "configuration")
endmacro()

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  check_config(FooConfig)
  check_config(BarConfig)
else()
  check_config(TestConfig)
endif()
