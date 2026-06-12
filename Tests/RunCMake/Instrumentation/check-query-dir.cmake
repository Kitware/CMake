include(${CMAKE_CURRENT_LIST_DIR}/verify-snippet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/validate_schema.cmake)

file(GLOB_RECURSE queries LIST_DIRECTORIES false ${v1}/query/*)
foreach(query ${queries})
  validate_schema(
    "${query}"
    "${CMAKE_CURRENT_LIST_DIR}/../../../Help/manual/instrumentation/query-v1-schema.json"
    "${schema_validate_result}"
  )
  if (RunCMake_TEST_FAILED)
    add_error("${RunCMake_TEST_FAILED}")
  endif()
endforeach()

if (ERROR_MESSAGE)
  message(FATAL_ERROR ${ERROR_MESSAGE})
endif()
