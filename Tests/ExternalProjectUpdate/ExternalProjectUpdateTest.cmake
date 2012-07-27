# Set the ExternalProject GIT_TAG to desired_tag, and make sure the
# resulting checked out version is resulting_sha and a rebuild.
# This check's the viability of the ExternalProject UPDATE_COMMAND.
macro(check_a_tag desired_tag resulting_sha)
  # Configure
  execute_process(COMMAND ${CMAKE_COMMAND}
    -G ${CMAKE_TEST_GENERATOR}
    -DTEST_GIT_TAG:STRING=${desired_tag}
    ${ExternalProjectUpdate_SOURCE_DIR}
    WORKING_DIRECTORY ${ExternalProjectUpdate_BINARY_DIR}
    RESULT_VARIABLE error_code
    )
  if(error_code)
    message(FATAL_ERROR "Could not configure the project.")
  endif()

  # Build
  execute_process(COMMAND ${CMAKE_COMMAND}
    --build ${ExternalProjectUpdate_BINARY_DIR}
    RESULT_VARIABLE error_code
    )
  if(error_code)
    message(FATAL_ERROR "Could not build the project.")
  endif()

  # Check the resulting SHA
  execute_process(COMMAND ${GIT_EXECUTABLE}
    rev-list --max-count=1 HEAD
    WORKING_DIRECTORY ${ExternalProjectUpdate_BINARY_DIR}/CMakeExternals/Source/TutorialStep1-GIT
    RESULT_VARIABLE error_code
    OUTPUT_VARIABLE tag_sha
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(error_code)
    message(FATAL_ERROR "Could not check the sha.")
  endif()

  if(NOT (${tag_sha} STREQUAL ${resulting_sha}))
    message(FATAL_ERROR "UPDATE_COMMAND produced
  ${tag_sha}
when
  ${resulting_sha}
was expected."
    )
  endif()
endmacro()

find_package(Git)
if(GIT_EXECUTABLE)
  check_a_tag(origin/master 5842b503ba4113976d9bb28d57b5aee1ad2736b7)
  check_a_tag(tag1          d1970730310fe8bc07e73f15dc570071f9f9654a)
  # With the Git UPDATE_COMMAND performance patch, this will not required a
  # 'git fetch'
  check_a_tag(tag1          d1970730310fe8bc07e73f15dc570071f9f9654a)
  check_a_tag(tag2          5842b503ba4113976d9bb28d57b5aee1ad2736b7)
  check_a_tag(d19707303     d1970730310fe8bc07e73f15dc570071f9f9654a)
  check_a_tag(origin/master 5842b503ba4113976d9bb28d57b5aee1ad2736b7)
  # This is a remote symbolic ref, so it will always trigger a 'git fetch'
  check_a_tag(origin/master 5842b503ba4113976d9bb28d57b5aee1ad2736b7)
endif(GIT_EXECUTABLE)
