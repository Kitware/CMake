include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

if (NOT IS_DIRECTORY "${v1}/data/content")
  add_error("Custom content directory does not exist.")
endif()

file(GLOB content_file ${v1}/data/content/*)
read_json("${content_file}" contents)
json_assert_key("${content_file}" "${contents}" custom "{}")
