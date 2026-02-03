include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

if (NOT IS_DIRECTORY "${v1}/data/content")
  add_error("Custom content directory does not exist.")
endif()

file(GLOB content_files ${v1}/data/content/*)
list(LENGTH content_files num)
if (NOT ${num} EQUAL 2)
  add_error("Found ${num} custom content files, expected 2.")
endif()

# Check contents of cmakeContent files
set(firstFile "")
foreach(content_file IN LISTS content_files)
  read_json("${content_file}" contents)

  # Check project name
  json_assert_key("${content_file}" "${contents}" project "instrumentation")

  # Check custom content
  string(JSON custom GET "${contents}" custom)
  json_assert_key("${content_file}" "${custom}" myString "string")
  json_assert_key("${content_file}" "${custom}" myBool "OFF")
  json_assert_key("${content_file}" "${custom}" myInt "1")
  json_assert_key("${content_file}" "${custom}" myFloat "2.5")
  json_assert_key("${content_file}" "${custom}" myTrue "ON")
  json_assert_key("${content_file}" "${custom}" myList "\\[ \"a\", \"b\", \"c\" \\]")
  json_assert_key("${content_file}" "${custom}" myObject "{.*\"key\".*:.*\"value\".*}")
  if (NOT firstFile)
    set(firstFile "${content_file}")
  endif()
  if ("${content_file}" STREQUAL "${firstFile}")
    string(JSON firstN GET "${custom}" nConfigure)
  else()
    string(JSON secondN GET "${custom}" nConfigure)
  endif()

  # Check target content
  string(JSON targets GET "${contents}" targets)
  string(JSON targetData GET "${targets}" lib)
  json_assert_key("${content_file}" "${targetData}" labels "\\[ \"label3\" \\]")
  json_assert_key("${content_file}" "${targetData}" type "STATIC_LIBRARY")

  string(JSON targetData GET "${targets}" main)
  json_assert_key("${content_file}" "${targetData}" labels "\\[ \"label1\", \"label2\" \\]")
  json_assert_key("${content_file}" "${targetData}" type "EXECUTABLE")

endforeach()

# Ensure provided -DN=* arguments result in differing JSON contents
math(EXPR expectedSecondN "3-${firstN}")
if (NOT ${secondN} EQUAL ${expectedSecondN})
  add_error("Configure content did not correspond to provided cache variables.\nGot: ${firstN} and ${secondN}")
endif()

# Ensure snippets reference valid files
foreach(snippet IN LISTS snippets)
  read_json("${snippet}" contents)
  string(JSON filename GET "${contents}" cmakeContent)
  if (NOT EXISTS "${v1}/data/${filename}")
    add_error("Reference to content file that does not exist.")
  endif()
endforeach()
