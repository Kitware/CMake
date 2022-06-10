add_executable(GlobalWithoutArgsKeywordExe IMPORTED)
set_target_properties(GlobalWithoutArgsKeywordExe PROPERTIES
  IMPORTED_LOCATION "${CMAKE_COMMAND}"
)

set(GlobalWithoutArgsKeyword_FOUND TRUE)
