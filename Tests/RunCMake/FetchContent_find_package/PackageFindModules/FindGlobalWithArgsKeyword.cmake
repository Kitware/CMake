add_executable(GlobalWithArgsKeywordExe IMPORTED)
set_target_properties(GlobalWithArgsKeywordExe PROPERTIES
  IMPORTED_LOCATION "${CMAKE_COMMAND}"
)

set(GlobalWithArgsKeyword_FOUND TRUE)
