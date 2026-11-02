set(CMAKE_C_STANDARD_LINK_DIRECTORIES "${CMAKE_BINARY_DIR}/child-only")
add_library(standard_link_archive STATIC library.c)
set_target_properties(standard_link_archive PROPERTIES
  OUTPUT_NAME standard_link_child_only
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/child-only/$<0:>")
add_executable(standard_link_child main.c)
set_target_properties(standard_link_child PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
# Deliberately use a plain library name, not a target or a full library path.
target_link_libraries(standard_link_child PRIVATE standard_link_child_only)
add_dependencies(standard_link_child standard_link_archive)
