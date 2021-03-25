function(findAttribute project attr)
  execute_process(
      COMMAND grep ${attr} ${RunCMake_TEST_BINARY_DIR}/${project}.xcodeproj/project.pbxproj
      OUTPUT_VARIABLE output_var
      RESULT_VARIABLE result_var
  )

  if(NOT result_var)
    set(RunCMake_TEST_FAILED "${attr} attribute is set" PARENT_SCOPE)
  endif()
endfunction()

findAttribute(${test} "RemoveHeadersOnCopy")
findAttribute(${test} "CodeSignOnCopy")
