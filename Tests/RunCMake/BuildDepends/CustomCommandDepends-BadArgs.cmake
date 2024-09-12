enable_language(C)

add_custom_command(OUTPUT main.c
  DEPFILE main.c.d
  IMPLICIT_DEPENDS C main.c.in
  COMMAND "${CMAKE_COMMAND}" -DINFILE=main.c.in -DOUTFILE=main.c -DDEPFILE=main.c.d
  -P "${CMAKE_CURRENT_SOURCE_DIR}/GenerateDepFile.cmake"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_target(mainc ALL DEPENDS main.c)
