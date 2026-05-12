# CMake should warn the user if they have a target named codegen... unless the
# user also suppresses the diagnostic.
cmake_diagnostic(SET CMD_AUTHOR IGNORE RECURSE)
add_custom_target(codegen
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/generated.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated.h
)
