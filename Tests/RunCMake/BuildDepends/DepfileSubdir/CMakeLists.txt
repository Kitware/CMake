cmake_policy(SET CMP0116 NEW)

add_custom_command(
  OUTPUT subcc.c
  DEPFILE subcc.c.d
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=${CMAKE_CURRENT_BINARY_DIR}/subcc.c -DINFILE=subccdep.txt -DDEPFILE=subcc.c.d -P "${CMAKE_CURRENT_LIST_DIR}/../WriteDepfile.cmake"
  )
add_custom_target(subcc ALL DEPENDS subcc.c)

add_custom_command(
  OUTPUT subexe.c
  DEPFILE subexe.c.d
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=subexe.c -DINFILE=subexedep.txt -DDEPFILE=subexe.c.d -P "${CMAKE_CURRENT_LIST_DIR}/../WriteDepfile.cmake"
  )
add_executable(subexe subexe.c)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/../sublib.c
  DEPFILE ${CMAKE_CURRENT_BINARY_DIR}/sublib.c.d
  COMMAND ${CMAKE_COMMAND} -DOUTFILE=${CMAKE_BINARY_DIR}/../sublib.c "-DINFILE=${CMAKE_CURRENT_BINARY_DIR}/sublibdep.txt" -DDEPFILE=sublib.c.d -P "${CMAKE_CURRENT_LIST_DIR}/../WriteDepfile.cmake"
  )
add_library(sublib STATIC "${CMAKE_BINARY_DIR}/../sublib.c")
