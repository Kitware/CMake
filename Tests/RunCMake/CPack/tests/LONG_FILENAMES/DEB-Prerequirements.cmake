function(get_test_prerequirements found_var)
  find_program(FAKEROOT_EXECUTABLE NAMES fakeroot)

  if(FAKEROOT_EXECUTABLE)
    set(${found_var} true PARENT_SCOPE)
  endif()
endfunction()
