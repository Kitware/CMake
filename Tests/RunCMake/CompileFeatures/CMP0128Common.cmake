enable_language(@lang@)

# Make sure the compile command is not hidden.
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_@lang@_COMPILE_OBJECT "${CMAKE_@lang@_COMPILE_OBJECT}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_@lang@_COMPILE_OBJECT "${CMAKE_@lang@_COMPILE_OBJECT}")

add_library(foo "@RunCMake_SOURCE_DIR@/empty.@ext@")
