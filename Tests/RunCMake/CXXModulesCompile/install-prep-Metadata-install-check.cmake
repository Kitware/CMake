set(metadata "${prefix}/lib/cmake/export_interfaces/cxx-modules/target-export__interfaces-Debug.cmake")
if (NOT EXISTS "${metadata}")
  list(APPEND RunCMake_TEST_FAILED "The prepared module export metadata was not installed.")
  return ()
endif ()

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E compare_files --ignore-eol
    "${CMAKE_CURRENT_LIST_DIR}/expect/install-prep-export-interfaces-Debug.cmake"
    "${metadata}"
  RESULT_VARIABLE result)
if (NOT result EQUAL 0)
  list(APPEND RunCMake_TEST_FAILED "Installed module export metadata does not match the expected file.")
endif ()
