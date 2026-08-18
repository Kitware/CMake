if(NOT DEFINED tests_file)
  message(FATAL_ERROR "tests_file not set")
endif()

if(NOT EXISTS "${tests_file}")
  message(FATAL_ERROR "Discovered tests file not found: ${tests_file}")
endif()

file(READ "${tests_file}" content)

if(NOT content MATCHES "_CMAKE_TEST_BUILD_DEPENDS"
   AND NOT content MATCHES "BUILD_DEPENDS")
  message(FATAL_ERROR
    "Expected discovered tests to contain BUILD_DEPENDS metadata.")
endif()

if(dependency_manifest)
  file(READ "${dependency_manifest}" dependencies)
  file(STRINGS "${tests_file}" metadata REGEX "BUILD_DEPENDS")
  foreach(dependency IN LISTS dependencies)
    set(count 0)
    foreach(line IN LISTS metadata)
      string(FIND "${line}" "${dependency}" pos)
      if(NOT pos EQUAL -1)
        math(EXPR count "${count} + 1")
      endif()
    endforeach()
    # Both add_test and discover_tests must record the object, not its target name.
    if(count LESS 2)
      message(FATAL_ERROR "Missing object file dependency metadata: ${dependency}")
    endif()
  endforeach()
endif()
