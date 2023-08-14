enable_language(C)

add_library (static1 STATIC empty.c)
set_property (TARGET static1 PROPERTY VERSION 2.5.0)
set_property (TARGET static1 PROPERTY SOVERSION 2.0.0)

file (GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/test.txt"
  CONTENT "[$<TARGET_SONAME_IMPORT_FILE:static1>]")
