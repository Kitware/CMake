file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ell.txt" "ell")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/rld.txt" "rld")
execute_process(
  COMMAND ${CMAKE_COMMAND} -E echo_append "H"
  COMMAND ${CMAKE_COMMAND} -E cat -
  )
execute_process(
  COMMAND ${CMAKE_COMMAND} -E echo_append "o wo"
  COMMAND ${CMAKE_COMMAND} -E cat "${CMAKE_CURRENT_BINARY_DIR}/ell.txt" - "${CMAKE_CURRENT_BINARY_DIR}/rld.txt"
  )
