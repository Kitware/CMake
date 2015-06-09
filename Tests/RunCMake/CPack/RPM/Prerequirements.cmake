function(get_test_prerequirements found_var config_file)
  find_program(RPM_EXECUTABLE rpm)

  if(RPM_EXECUTABLE)
    file(WRITE "${config_file}" "set(RPM_EXECUTABLE \"${RPM_EXECUTABLE}\")")
    set(${found_var} true PARENT_SCOPE)
  endif()
endfunction()
