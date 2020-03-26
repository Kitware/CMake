function(get_test_prerequirements found_var config_file)
  find_program(HDIUTIL_EXECUTABLE hdiutil)

  if(HDIUTIL_EXECUTABLE)
    file(WRITE "${config_file}" "set(HDIUTIL_EXECUTABLE \"${HDIUTIL_EXECUTABLE}\")")
    set(${found_var} true PARENT_SCOPE)
  endif()
endfunction()
