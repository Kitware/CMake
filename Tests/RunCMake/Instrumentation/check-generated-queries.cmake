include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

foreach(n IN LISTS generated_queries)
  set(expected_file "${RunCMake_TEST_BINARY_DIR}/query/query-${n}.json")
  set(generated_file "${v1}/query/generated/query-${n}.json")
  json_equals(${expected_file} ${generated_file})
endforeach()
