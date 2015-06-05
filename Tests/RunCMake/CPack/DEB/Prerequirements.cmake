function(get_test_prerequirements found_var config_file)
  find_program(DPKG_EXECUTABLE dpkg)

  if(DPKG_EXECUTABLE)
    file(WRITE "${config_file}" "set(DPKG_EXECUTABLE \"${DPKG_EXECUTABLE}\")")
    set(${found_var} true PARENT_SCOPE)
  endif()
endfunction()
