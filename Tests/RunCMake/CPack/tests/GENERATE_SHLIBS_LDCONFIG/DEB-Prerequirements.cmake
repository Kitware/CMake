function(get_test_prerequirements found_var config_file)
  include(${config_file})

  if(CPACK_READELF_EXECUTABLE)
    set(${found_var} true PARENT_SCOPE)
  endif()
endfunction()
