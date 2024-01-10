
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/dummy.txt"
  COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/dummy.txt"
  DEPFILE dummy.txt.d
)

add_custom_target(subdir_space ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/dummy.txt"
)
