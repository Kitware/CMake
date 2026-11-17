# The root utility output collides with its alias.  A subdirectory utility
# has a distinct alias, which must remain available in the enclosing build.
add_custom_target(probe
  COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_BINARY_DIR}/probe-$<CONFIG>")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/nested-src/CMakeLists.txt" [[
add_custom_target(distinct
  COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_BINARY_DIR}/distinct-$<CONFIG>")
]])
add_subdirectory("${CMAKE_CURRENT_BINARY_DIR}/nested-src" nested)
