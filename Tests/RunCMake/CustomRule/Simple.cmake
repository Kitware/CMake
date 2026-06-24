
enable_language(C)

add_custom_rule(simple OUTPUT <CURRENT_BINARY_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E copy <SOURCE> <CURRENT_BINARY_DIR>
  COMMAND "${CMAKE_COMMAND}" -E rename <CURRENT_BINARY_DIR>/<FILE_NAME> <CURRENT_BINARY_DIR>/<BASE_NAME>.c)

add_library(foo STATIC)

target_sources(foo PRIVATE FILE_SET fs TYPE simple FILES file.i)
