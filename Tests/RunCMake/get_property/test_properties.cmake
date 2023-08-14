function (check_test_property test prop dir)
  set(dir_args)
  if(dir)
    set(dir_args DIRECTORY ${dir})
  endif()
  get_test_property("${test}" "${prop}" ${dir_args} gtp_val)
  get_property(gp_val
    TEST "${test}" ${dir_args}
    PROPERTY "${prop}")

  message("get_test_property: -->${gtp_val}<--")
  message("get_property: -->${gp_val}<--")
endfunction ()

include(CTest)
add_test(NAME test COMMAND "${CMAKE_COMMAND}" --help)
set_tests_properties(test PROPERTIES empty "" custom value)
add_subdirectory(test_properties)

check_test_property(test empty "")
check_test_property(test custom "")
check_test_property(test noexist "")
check_test_property(test custom test_properties)
check_test_property(test custom ${CMAKE_BINARY_DIR}/test_properties)
