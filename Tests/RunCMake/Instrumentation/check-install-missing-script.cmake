include(${CMAKE_CURRENT_LIST_DIR}/json.cmake)

# The install failed before running any script, but the cmakeInstall snippet
# should still be written, recording the failure.
file(GLOB snippets LIST_DIRECTORIES false ${v1}/data/cmakeInstall-*.json)
list(LENGTH snippets num)
if (NOT num EQUAL 1)
  add_error("Expected exactly one cmakeInstall snippet, found ${num}: ${snippets}")
else()
  read_json("${snippets}" contents)
  string(JSON result GET "${contents}" result)
  if (result EQUAL 0)
    add_error("Expected nonzero result in cmakeInstall snippet:\n${contents}")
  endif()
endif()
