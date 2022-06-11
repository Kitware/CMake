add_executable(LocalWithArgsKeywordExe IMPORTED)
set_target_properties(LocalWithArgsKeywordExe PROPERTIES
  IMPORTED_LOCATION "${CMAKE_COMMAND}"
)

set(LocalWithArgsKeyword_FOUND TRUE)
