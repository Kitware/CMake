include(${CMAKE_CURRENT_LIST_DIR}/verify-snippet.cmake)

if (NOT IS_DIRECTORY "${v1}/data/content")
  add_error("Custom content directory does not exist.")
endif()

file(GLOB content_files ${v1}/data/content/*)
list(LENGTH content_files num)
if (NOT ${num} EQUAL 1)
  add_error("Found ${num} custom content files, expected 1.")
endif()
