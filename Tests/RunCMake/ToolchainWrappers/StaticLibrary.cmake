include(common.cmake)
enable_language(C)
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_CREATE_STATIC_LIBRARY "${CMAKE_C_CREATE_STATIC_LIBRARY}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_CREATE_STATIC_LIBRARY "${CMAKE_C_CREATE_STATIC_LIBRARY}")

# Static libraries are created by the archiver: compile wrappers apply, but
# link wrappers and linker launchers do not.
set(CMAKE_C_LINKER_LAUNCHER "${CMAKE_COMMAND};-E;env;LL_LAUNCH=1")
add_library(main STATIC main.c)
set_target_properties(main PROPERTIES
  COMPILE_WRAPPERS "EXAMPLE_1;EXAMPLE_2"
  LINK_WRAPPERS "EXAMPLE_1;EXAMPLE_2"
)
