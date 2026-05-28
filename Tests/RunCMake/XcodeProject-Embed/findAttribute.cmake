cmake_policy(VERSION 3.1...3.20)

function(findAttribute project attr expectPresent)
  set(expectCount ${ARGV3})

  if(NOT expectPresent)
    set(expectCount 0)
  endif()

  execute_process(
      COMMAND grep -c ${attr} ${RunCMake_TEST_BINARY_DIR}/${project}.xcodeproj/project.pbxproj
      OUTPUT_VARIABLE actualCount
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  if(NOT actualCount MATCHES "^[0-9]+$")
    set(actualCount 0)
  endif()

  if(NOT ("${expectCount}" STREQUAL "") AND NOT (actualCount EQUAL expectCount))
    set(RunCMake_TEST_FAILED "${attr} expected ${expectCount} matches, but found ${actualCount}" PARENT_SCOPE)
  endif()
endfunction()
