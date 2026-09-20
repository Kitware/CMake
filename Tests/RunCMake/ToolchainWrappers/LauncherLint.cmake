include(common.cmake)
enable_language(C)

set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_COMMAND};-E;env;CL_LAUNCH=1")
# Lint tools run the wrappers and launcher via `__run_co_compile --launcher=`.
# `cmake -E true` ignores the arguments it is given.
set(CMAKE_C_CPPCHECK "${CMAKE_COMMAND};-E;true")
example_exe(main.c)
