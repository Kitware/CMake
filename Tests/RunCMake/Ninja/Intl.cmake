include(Intl-common.cmake)
set(input "${CMAKE_CURRENT_BINARY_DIR}/${intl}-input.txt")
set(output "${CMAKE_CURRENT_BINARY_DIR}/${intl}-output.txt")
file(WRITE "${input}" "${intl}\n")
add_custom_command(OUTPUT "${output}"
  COMMAND ${CMAKE_COMMAND} -E copy "${input}" "${output}")
add_custom_target(drive ALL DEPENDS "${output}")
