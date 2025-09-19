include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

if (NOT IS_DIRECTORY "${v1}/data/content")
  add_error("Custom content directory does not exist.")
endif()

file(GLOB content_files ${v1}/data/content/*)
list(LENGTH content_files num)
if (NOT ${num} EQUAL ${EXPECTED_CONTENT_FILES})
  add_error("Found ${num} custom content files, expected ${EXPECTED_CONTENT_FILES}.")
endif()
