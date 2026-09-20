set(CTEST_USE_LAUNCHERS 1)
include(CTestUseLaunchers)
include(common.cmake)
enable_language(C)

# The `ctest --launch` launcher is outside of wrappers and launchers.
set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_COMMAND};-E;env;CL_LAUNCH=1")
set(CMAKE_C_LINKER_LAUNCHER "${CMAKE_COMMAND};-E;env;LL_LAUNCH=1")
example_exe(main.c)
