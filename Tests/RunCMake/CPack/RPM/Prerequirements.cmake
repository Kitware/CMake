function(get_test_prerequirements found_var config_file)
  if(CMAKE_CURRENT_BINARY_DIR MATCHES " ")
    # rpmbuild can't handle spaces in path
    return()
  endif()

  find_program(RPM_EXECUTABLE rpm)
  find_program(RPMBUILD_EXECUTABLE rpmbuild)

  if(RPM_EXECUTABLE AND RPMBUILD_EXECUTABLE)
    file(WRITE "${config_file}" "set(RPM_EXECUTABLE \"${RPM_EXECUTABLE}\")")
    file(APPEND "${config_file}"
        "\nset(RPMBUILD_EXECUTABLE \"${RPMBUILD_EXECUTABLE}\")")
    set(${found_var} true PARENT_SCOPE)
  endif()

  # optional tool for some tests
  find_program(CPACK_OBJDUMP_EXECUTABLE objdump)
  if(CPACK_OBJDUMP_EXECUTABLE)
    file(APPEND "${config_file}"
      "\nset(CPACK_OBJDUMP_EXECUTABLE \"${CPACK_OBJDUMP_EXECUTABLE}\")")
  endif()
endfunction()
