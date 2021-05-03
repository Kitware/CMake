enable_language(C)

add_executable(generator main.c)

string(REPEAT "." 5000 very_long)

add_custom_command(
  OUTPUT gen.txt
  COMMAND generator "${very_long}" > gen.txt
  )

add_custom_target(
  custom
  ALL
  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/gen.txt"
  )

add_executable(exe main.c)

add_custom_command(
  TARGET exe POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Post-build $<CONFIG> $<COMMAND_CONFIG:$<CONFIG>> ${very_long}"
  )
