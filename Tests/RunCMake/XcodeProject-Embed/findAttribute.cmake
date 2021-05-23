cmake_policy(VERSION 3.1...3.20)

function(findAttribute project attr expectPresent)
  execute_process(
      COMMAND grep ${attr} ${RunCMake_TEST_BINARY_DIR}/${project}.xcodeproj/project.pbxproj
      OUTPUT_VARIABLE output_var
      RESULT_VARIABLE result_var
  )

  if(${expectPresent})
    if(result_var)
      set(RunCMake_TEST_FAILED "${attr} attribute is not set" PARENT_SCOPE)
    endif()
  else()
    if(NOT result_var)
      set(RunCMake_TEST_FAILED "${attr} attribute is set" PARENT_SCOPE)
    endif()
  endif()
endfunction()
