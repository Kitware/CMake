set(installBase ${RunCMake_TEST_BINARY_DIR}/root-all)

foreach(i RANGE 1 5)
  set(subdir shouldNotRemain${i})
  if(IS_DIRECTORY ${installBase}/${subdir})
    set(RunCMake_TEST_FAILED "Check failed.")
    string(APPEND RunCMake_TEST_FAILURE_MESSAGE
      "\nUnexpectedly created install path that should have disappeared with "
      "normalization:\n"
      "  ${installBase}/${subdir}"
    )
  endif()
endforeach()

file(GLOB perConfigFiles ${installBase}/lib/cmake/pkg/pkg-config-*.cmake)
foreach(file IN LISTS perConfigFiles ITEMS ${installBase}/lib/cmake/pkg/pkg-config.cmake)
  file(STRINGS ${file} matches REGEX shouldNotRemain)
  if(NOT matches STREQUAL "")
    set(RunCMake_TEST_FAILED "Check failed.")
    string(APPEND RunCMake_TEST_FAILURE_MESSAGE
      "\nNon-normalized path found in ${file}:"
    )
    foreach(match IN LISTS matches)
      string(APPEND RunCMake_TEST_FAILURE_MESSAGE "\n  ${match}")
    endforeach()
  endif()
endforeach()

if(NOT EXISTS "${installBase}/dirs/dir/empty.txt")
  set(RunCMake_TEST_FAILED "Check failed.")
  string(APPEND RunCMake_TEST_FAILURE_MESSAGE
    "\nNon-normalized DIRECTORY destination not handled correctly. "
    "Expected to find the following file, but it was missing:\n"
    "  ${installBase}/dirs/dir/empty.txt"
  )
endif()
