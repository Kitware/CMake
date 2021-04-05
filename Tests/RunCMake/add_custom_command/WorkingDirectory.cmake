add_custom_target(mkdir COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
add_custom_command(
  OUTPUT out.txt
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/PrintDir.cmake
  WORKING_DIRECTORY ${CMAKE_CFG_INTDIR}
  )
set_property(SOURCE out.txt PROPERTY SYMBOLIC 1)
add_custom_target(drive ALL DEPENDS out.txt)
add_dependencies(drive mkdir)
