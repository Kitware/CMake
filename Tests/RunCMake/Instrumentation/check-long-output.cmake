include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

if(NOT EXISTS "${RunCMake_TEST_BINARY_DIR}/Destination-Modules/check-done.txt")
  string(APPEND RunCMake_TEST_FAILED
    "long custom command with instrumentation did not run (Destination-Modules missing)\n")
endif()

# Reconstruct expected outputs as in project/CMakeLists.txt
file(GLOB_RECURSE input_files
  RELATIVE "${CMAKE_ROOT}"
  "${CMAKE_ROOT}/Modules/*")
set(expected_outs)
foreach(f IN LISTS input_files)
  list(APPEND expected_outs "Destination-${f}")
endforeach()
list(LENGTH expected_outs expected_count)
list(SORT expected_outs)

# Find the custom snippet for the long-output command and verify its outputs
file(GLOB snippets LIST_DIRECTORIES false "${v1}/data/custom-*")
set(found_long_snippet 0)
foreach(snippet IN LISTS snippets)
  read_json("${snippet}" contents)
  string(JSON outputs ERROR_VARIABLE noOutputs GET "${contents}" outputs)
  if(NOT outputs MATCHES "Destination-")
    continue()
  endif()
  set(found_long_snippet 1)

  string(JSON outputs_len LENGTH "${contents}" outputs)
  if(NOT outputs_len EQUAL expected_count)
    json_error("${snippet}"
      "Expected ${expected_count} outputs, got ${outputs_len}")
    continue()
  endif()

  set(actual_outs)
  math(EXPR last "${outputs_len} - 1")
  foreach(i RANGE ${last})
    string(JSON out GET "${contents}" outputs ${i})
    string(REPLACE "\\" "/" out "${out}")
    list(APPEND actual_outs "${out}")
  endforeach()
  list(SORT actual_outs)
  if(NOT expected_outs STREQUAL actual_outs)
    json_error("${snippet}"
      "outputs field does not match expected Destination-* list")
  endif()
endforeach()

if(NOT found_long_snippet)
  add_error("No custom snippet with Destination- outputs was found")
endif()
