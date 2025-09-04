include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

if (NOT IS_DIRECTORY "${v1}/data/trace")
  add_error("Trace directory ${v1}/data/trace does not exist.")
endif()

file(GLOB trace_files ${v1}/data/trace/*)
list(LENGTH trace_files num)
if (NOT ${num} EQUAL 1)
  add_error("Found ${num} trace files, expected 1.")
endif()
