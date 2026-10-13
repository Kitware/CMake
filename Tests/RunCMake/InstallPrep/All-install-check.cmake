set(_check_files
  [[bin]]
  [[bin/imported-tool(\.exe)?]]
  [[bin/runtime(\.exe)?]]
  [[lib]]
  [[lib/(lib)?default_component\.(a|lib|l)]]
  [[lib/(lib)?development\.(a|lib|l)]]
  [[share]]
  [[share/install-prep]]
  [[share/install-prep/lib\.c]])
check_installed("^${_check_files}$")

foreach(program IN ITEMS runtime imported-tool)
  execute_process(COMMAND "${CMAKE_INSTALL_PREFIX}/bin/${program}${CMAKE_HOST_EXECUTABLE_SUFFIX}"
    RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    set(RunCMake_TEST_FAILED "Installed ${program} failed: ${result}")
  endif()
endforeach()
