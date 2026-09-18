include(${CMAKE_CURRENT_LIST_DIR}/verify-snippet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../validate_json_schema.cmake)

set(schema_file "${CMAKE_CURRENT_LIST_DIR}/../../../Help/manual/instrumentation/query-v1-schema.json")

file(GLOB_RECURSE queries LIST_DIRECTORIES false ${v1}/query/*)
foreach(query ${queries})
  block(SCOPE_FOR VARIABLES PROPAGATE schema_error)
    # Capture only the error message from this validate_json_chema call
    set(RunCMake_TEST_FAILED "")
    validate_json_schema(
      "${schema_file}" "${query}"
      EXPECTED_RESULT "${schema_validate_result}"
    )
    set(schema_error "${RunCMake_TEST_FAILED}")
  endblock()
  if (schema_error)
    add_error("${schema_error}")
  endif()
endforeach()

if (ERROR_MESSAGE)
  message(FATAL_ERROR "${ERROR_MESSAGE}")
endif()
