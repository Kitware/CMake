enable_language(C)

add_custom_command(OUTPUT main.c
  DEPFILE main.c.d
  COMMAND "${CMAKE_COMMAND}" -DINFILE=${CMAKE_CURRENT_SOURCE_DIR}/main.c -DOUTFILE=main.c -DDEPFILE=main.c.d
  -P "${CMAKE_CURRENT_SOURCE_DIR}/GenerateDepFile.cmake"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

add_executable(main ${CMAKE_CURRENT_BINARY_DIR}/main.c)
