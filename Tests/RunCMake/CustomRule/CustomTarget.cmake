
enable_language(C)

add_custom_rule(simple OUTPUT <CURRENT_BINARY_DIR>/<FILE_NAME>
  COMMAND "${CMAKE_COMMAND}" -E copy <SOURCE> <CURRENT_BINARY_DIR>)

add_custom_target(foo ALL)

target_sources(foo PRIVATE FILE_SET fs TYPE simple FILES file.i)
