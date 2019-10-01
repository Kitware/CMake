if(EXISTS ${RunCMake_TEST_BINARY_DIR}/check-debug.cmake)
  include(${RunCMake_TEST_BINARY_DIR}/check-debug.cmake)
  if(RunCMake_TEST_FAILED)
    return()
  endif()

  foreach(file
      "${foo_lib}"
      "${subinc_lib}"
      "${zot_lib}"
      "${subsubinc_lib}"
      )
    if(NOT EXISTS "${file}")
      set(RunCMake_TEST_FAILED
        "Artifact should exist but is missing:\n  ${file}")
      return()
    endif()
  endforeach()
  foreach(file
      "${main_exe}"
      "${bar_lib}"
      "${subsub_lib}"
      )
    if(EXISTS "${file}")
      set(RunCMake_TEST_FAILED
        "Artifact should be missing but exists:\n  ${file}")
      return()
    endif()
  endforeach()
else()
  set(RunCMake_TEST_FAILED "
 '${RunCMake_TEST_BINARY_DIR}/check-debug.cmake' missing
")
endif()
