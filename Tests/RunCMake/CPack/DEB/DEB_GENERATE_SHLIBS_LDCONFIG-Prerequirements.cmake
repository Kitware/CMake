function(get_test_prerequirements found_var)
  find_program(READELF_EXECUTABLE NAMES readelf)

  if(READELF_EXECUTABLE)
    set(${found_var} true PARENT_SCOPE)
  endif()
endfunction()
