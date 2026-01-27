cmake_minimum_required(VERSION 3.12)

function(run_git exe exe_display)
  execute_process(COMMAND ${exe} --version
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE result
    )

  if(NOT result EQUAL 0)
    message(SEND_ERROR "Result of ${exe_display} --version is ${result}, should be 0")
  endif()

  if(NOT output STREQUAL "git version ${Git_VERSION}")
    message(SEND_ERROR "Output of ${exe_display} --version is \"${output}\", should be \"git version ${Git_VERSION}\"")
  endif()

  if(NOT Git_VERSION VERSION_EQUAL GIT_VERSION_STRING)
    message(SEND_ERROR "Git_VERSION=\"${Git_VERSION}\" is not equal to GIT_VERSION_STRING=\"${GIT_VERSION_STRING}\"")
  endif()
endfunction()

run_git("${GIT_EXECUTABLE}" "\${GIT_EXECUTABLE}")
run_git("${GIT_EXECUTABLE_TARGET}" "Git::Git")
