include(${CMAKE_CURRENT_LIST_DIR}/Assertions.cmake)

set(out_dir "${RunCMake_BINARY_DIR}/PerConfigGeneration-build/CMakeFiles/Export/510c5684a4a8a792eadfb55bc9744983")

macro(check_config)
  string(TOLOWER "${config}" config_lower)
  if(NOT EXISTS "${out_dir}/foo@${config_lower}.cps")
    set(RunCMake_TEST_FAILED
      "Configuration file for '${config}' does not exist")
    return()
  endif()

  file(READ "${out_dir}/foo@${config_lower}.cps" content)
  expect_value("${content}" "${config}" "configuration")
endmacro()

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    check_config()
  endforeach()
else()
  include(${RunCMake_TEST_BINARY_DIR}/build_type.cmake)
  check_config()
endif()
