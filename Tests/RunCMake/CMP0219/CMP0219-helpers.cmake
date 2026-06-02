function(cmp0219_assert_equal actual expected)
  if(NOT "${actual}" STREQUAL "${expected}")
    message(FATAL_ERROR
      "Assertion failed:\n"
      "  expected=[${expected}]\n"
      "  actual=[${actual}]")
  endif()
endfunction()

function(cmp0219_assert_undefined variable_name)
  if(DEFINED ${variable_name})
    message(FATAL_ERROR
      "Assertion failed:\n"
      "  ${variable_name} should be undefined\n"
      "  actual=[${${variable_name}}]")
  endif()
endfunction()

set(cmp0219_path_fwd "C:/build_bot/new/temp/vendor")
string(REPLACE "/" "\\" cmp0219_path_native "${cmp0219_path_fwd}")
