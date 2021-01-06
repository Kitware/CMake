add_custom_command(OUTPUT out.txt
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/RepeatCMake-Custom-Script.cmake
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/RepeatCMake-Custom-Script.cmake
  )
add_custom_target(drive ALL DEPENDS out.txt)
