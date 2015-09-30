function(get_test_prerequirements found_var config_file)
  file(WRITE "${config_file}" "set(TAR_EXECUTABLE \"${CMAKE_COMMAND}\" -E tar)")
  set(${found_var} true PARENT_SCOPE)
endfunction()
