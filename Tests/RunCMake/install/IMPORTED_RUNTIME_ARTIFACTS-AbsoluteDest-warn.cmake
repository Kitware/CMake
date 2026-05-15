enable_language(C)
add_executable(myexe IMPORTED)
set_target_properties(myexe PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/myexe")
install(IMPORTED_RUNTIME_ARTIFACTS myexe RUNTIME DESTINATION /absolute/path)
