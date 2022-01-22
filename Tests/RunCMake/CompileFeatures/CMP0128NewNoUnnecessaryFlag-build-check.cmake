foreach(flag @flags@)
  string(FIND "${actual_stdout}" "${flag}" position)

  if(NOT position EQUAL -1)
    set(RunCMake_TEST_FAILED "\"${flag}\" compile flag found.")
    break()
  endif()
endforeach()
