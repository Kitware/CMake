foreach(arg
    RunCMake_GENERATOR
    RunCMake_SOURCE_DIR
    RunCMake_BINARY_DIR
    )
  if(NOT DEFINED ${arg})
    message(FATAL_ERROR "${arg} not given!")
  endif()
endforeach()

function(run_cmake test)
  set(top_src "${RunCMake_SOURCE_DIR}")
  set(top_bin "${RunCMake_BINARY_DIR}")
  if(EXISTS ${top_src}/${test}-result.txt)
    file(READ ${top_src}/${test}-result.txt expect_result)
    string(REGEX REPLACE "\n+$" "" expect_result "${expect_result}")
  else()
    set(expect_result 0)
  endif()
  foreach(o out err)
    if(EXISTS ${top_src}/${test}-std${o}.txt)
      file(READ ${top_src}/${test}-std${o}.txt expect_std${o})
      string(REGEX REPLACE "\n+$" "" expect_std${o} "${expect_std${o}}")
    else()
      unset(expect_std${o})
    endif()
  endforeach()
  set(RunCMake_TEST_SOURCE_DIR "${top_src}")
  set(RunCMake_TEST_BINARY_DIR "${top_bin}/${test}-build")
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")
  if(NOT DEFINED RunCMake_TEST_OPTIONS)
    set(RunCMake_TEST_OPTIONS "")
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} "${RunCMake_TEST_SOURCE_DIR}"
              -G "${RunCMake_GENERATOR}" -DRunCMake_TEST=${test}
              ${RunCMake_TEST_OPTIONS}
    WORKING_DIRECTORY "${RunCMake_TEST_BINARY_DIR}"
    OUTPUT_VARIABLE actual_stdout
    ERROR_VARIABLE actual_stderr
    RESULT_VARIABLE actual_result
    )
  set(msg "")
  if(NOT "${actual_result}" STREQUAL "${expect_result}")
    set(msg "${msg}Result is [${actual_result}], not [${expect_result}].\n")
  endif()
  foreach(o out err)
    string(REGEX REPLACE "\n+$" "" actual_std${o} "${actual_std${o}}")
    set(expect_${o} "")
    if(DEFINED expect_std${o})
      if(NOT "${actual_std${o}}" MATCHES "${expect_std${o}}")
        string(REGEX REPLACE "\n" "\n expect-${o}> " expect_${o}
          " expect-${o}> ${expect_std${o}}")
        set(expect_${o} "Expected std${o} to match:\n${expect_${o}}\n")
        set(msg "${msg}std${o} does not match that expected.\n")
      endif()
    endif()
  endforeach()
  unset(RunCMake_TEST_FAILED)
  include(${top_src}/${test}-check.cmake OPTIONAL)
  if(RunCMake_TEST_FAILED)
    set(msg "${RunCMake_TEST_FAILED}\n${msg}")
  endif()
  if(msg)
    string(REGEX REPLACE "\n" "\n actual-out> " actual_out " actual-out> ${actual_stdout}")
    string(REGEX REPLACE "\n" "\n actual-err> " actual_err " actual-err> ${actual_stderr}")
    message(SEND_ERROR "${test} - FAILED:\n"
      "${msg}"
      "${expect_out}"
      "Actual stdout:\n${actual_out}\n"
      "${expect_err}"
      "Actual stderr:\n${actual_err}\n"
      )
  else()
    message(STATUS "${test} - PASSED")
  endif()
endfunction()
