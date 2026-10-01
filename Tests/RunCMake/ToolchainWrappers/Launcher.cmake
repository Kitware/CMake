include(common.cmake)
enable_language(C)

# Wrappers go outside of launchers.
set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_COMMAND};-E;env;CL_LAUNCH=1")
set(CMAKE_C_LINKER_LAUNCHER "${CMAKE_COMMAND};-E;env;LL_LAUNCH=1")
example_exe(main.c)
