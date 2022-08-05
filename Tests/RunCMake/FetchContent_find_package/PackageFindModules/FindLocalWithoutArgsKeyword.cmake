add_executable(LocalWithoutArgsKeywordExe IMPORTED)
set_target_properties(LocalWithoutArgsKeywordExe PROPERTIES
  IMPORTED_LOCATION "${CMAKE_COMMAND}"
)

set(LocalWithoutArgsKeyword_FOUND TRUE)
