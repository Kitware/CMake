check_installed([[^bin;bin/runtime(\.exe)?$]])

execute_process(COMMAND "${CMAKE_INSTALL_PREFIX}/bin/runtime${CMAKE_HOST_EXECUTABLE_SUFFIX}"
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  set(RunCMake_TEST_FAILED "Installed runtime failed: ${result}")
endif()
