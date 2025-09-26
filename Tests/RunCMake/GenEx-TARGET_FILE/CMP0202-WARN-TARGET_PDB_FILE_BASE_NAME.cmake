
cmake_minimum_required(VERSION 4.0)

enable_language(C)

add_library(empty SHARED empty.c)

file(GENERATE
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test.txt"
  CONTENT "[$<TARGET_PDB_FILE_BASE_NAME:empty,POSTFIX:INCLUDE>]"
)
