cmake_policy(SET CMP0112 NEW)

enable_language (C)

add_executable (exec1 empty.c)

add_custom_target(copyFile
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/empty.c"
        "$<TARGET_FILE_DIR:exec1>/$<TARGET_FILE_BASE_NAME:exec1>_e.c"
)
add_dependencies(exec1 copyFile)
