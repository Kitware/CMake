# Relative BUILD_DEPENDS files are interpreted with respect to this directory:
# the source directory if the file exists there, and otherwise the binary
# directory.
add_custom_command(OUTPUT relative-file.txt
  COMMAND ${CMAKE_COMMAND} -E touch relative-file.txt
)
add_custom_target(generate_relative_file DEPENDS relative-file.txt)
install(CODE "
  if(NOT EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/relative-file.txt\")
    message(FATAL_ERROR \"Preparation did not generate relative-file.txt\")
  endif()
  "
  COMPONENT Payload
  BUILD_DEPENDS relative-file.txt CMakeLists.txt
)
