check_installed([[^bin;bin/imported-tool(\.exe)?$]])

execute_process(COMMAND "${CMAKE_INSTALL_PREFIX}/bin/imported-tool${CMAKE_HOST_EXECUTABLE_SUFFIX}"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  set(RunCMake_TEST_FAILED "Installed imported-tool failed: ${result}")
endif()
