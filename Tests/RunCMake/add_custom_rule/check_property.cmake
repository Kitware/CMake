
function(check_rule_property rule property expected)
  get_property(value RULE ${rule} PROPERTY ${property})
  if(NOT "${value}" STREQUAL "${expected}")
    string (APPEND TEST_FAILED "RULE ${rule}, PROPERTY ${property}: actual result:\n [${value}]\nbut expected:\n [${expected}]\n")
  endif()
  return(PROPAGATE TEST_FAILED)
endfunction()
