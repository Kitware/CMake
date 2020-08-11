enable_language(C)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

file(TOUCH ${CMAKE_BINARY_DIR}/empty.cmake)
include(${CMAKE_BINARY_DIR}/empty.cmake)

add_subdirectory(SimpleSubdir)

add_library(simplestatic STATIC simplelib.c)

include(${CMAKE_CURRENT_LIST_DIR}/Common.cmake)
generate_output_files(simpleexe simpleshared simplestatic simpleobj)

file(APPEND "${CMAKE_BINARY_DIR}/target_files.cmake" "set(GENERATED_FILES [==[${CMAKE_BINARY_DIR}/empty.cmake]==])\n")
