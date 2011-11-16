get_filename_component(CMakeTests_SRC_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
function(check_cmake_test prefix)
  get_filename_component(CMakeTests_BIN_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
  foreach(test ${ARGN})
    message(STATUS "Test ${prefix}-${test}...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -P "${CMakeTests_SRC_DIR}/${prefix}-${test}.cmake"
      WORKING_DIRECTORY "${CMakeTests_BIN_DIR}"
      OUTPUT_VARIABLE stdout
      ERROR_VARIABLE stderr
      RESULT_VARIABLE result
      )
    string(REGEX REPLACE "\n" "\n out> " out " out> ${stdout}")
    string(REGEX REPLACE "\n" "\n err> " err " err> ${stderr}")
    if(NOT "${result}" STREQUAL "${${test}-RESULT}")
      message(FATAL_ERROR
        "Test ${test} result is [${result}], not [${${test}-RESULT}].\n"
        "Test ${test} output:\n"
        "${out}\n"
        "${err}")
    endif()
    if(${test}-STDERR AND NOT "${err}" MATCHES "${${test}-STDERR}")
      message(FATAL_ERROR
        "Test ${test} stderr does not match\n  ${${test}-STDERR}\n"
        "Test ${test} output:\n"
        "${out}\n"
        "${err}")
    endif()
  endforeach()
endfunction()
