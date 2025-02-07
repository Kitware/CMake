include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)
function(check_generated_json n)
  set(expected_file "${RunCMake_TEST_BINARY_DIR}/query/query-${n}.json")
  set(generated_file "${v1}/query/generated/query-${n}.json")
  read_json("${expected_file}" expected)
  read_json("${generated_file}" generated)
  string(JSON equal EQUAL ${expected} ${generated})
  if (NOT equal)
    set(RunCMake_TEST_FAILED
      "Generated JSON ${generated}\nNot equal to expected ${expected}"
    )
  endif()
  return(PROPAGATE RunCMake_TEST_FAILED)
endfunction()

foreach(n IN LISTS generated_queries)
  check_generated_json(${n})
endforeach()
