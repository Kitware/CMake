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
