macro(_expect entity op actual expected)
  if(NOT "${actual}" ${op} "${expected}")
    list(JOIN ARGN "." name)
    set(RunCMake_TEST_FAILED
      "Attribute '${name}' ${entity} '${actual}' does not match expected ${entity} '${expected}'" PARENT_SCOPE)
    return()
  endif()
endmacro()

function(expect_value content expected_value)
  string(JSON actual_value GET "${content}" ${ARGN})
  _expect("value" STREQUAL "${actual_value}" "${expected_value}" ${ARGN})
endfunction()

function(expect_array content expected_length)
  string(JSON actual_type TYPE "${content}" ${ARGN})
  _expect("type" STREQUAL "${actual_type}" "ARRAY" ${ARGN})

  string(JSON actual_length LENGTH "${content}" ${ARGN})
  _expect("length" EQUAL "${actual_length}" "${expected_length}" ${ARGN})
endfunction()

function(expect_null content)
  string(JSON actual_type TYPE "${content}" ${ARGN})
  _expect("type" STREQUAL "${actual_type}" "NULL" ${ARGN})
endfunction()

function(expect_missing content)
  string(JSON value ERROR_VARIABLE error GET "${content}" ${ARGN})
  if(NOT value MATCHES "^(.*-)?NOTFOUND$")
    set(RunCMake_TEST_FAILED
      "Attribute '${ARGN}' is unexpectedly present" PARENT_SCOPE)
  endif()
endfunction()
