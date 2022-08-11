add_executable(EventuallyGlobalExe IMPORTED)
set_target_properties(EventuallyGlobalExe PROPERTIES
  IMPORTED_LOCATION "${CMAKE_COMMAND}"
)

set(EventuallyGlobal_FOUND TRUE)
