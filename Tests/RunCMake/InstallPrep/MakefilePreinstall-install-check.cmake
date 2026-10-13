check_installed([[^bin;bin/relink$]])

foreach(program IN ITEMS "${RunCMake_TEST_BINARY_DIR}/relink" "${CMAKE_INSTALL_PREFIX}/bin/relink")
  execute_process(COMMAND "${program}" RESULT_VARIABLE result)
  if(NOT result EQUAL 0)
    set(RunCMake_TEST_FAILED "Relinked executable ${program} failed: ${result}")
  endif()
endforeach()
