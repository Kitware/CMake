# Lint tools run the launcher through "cmake -E __run_co_compile --launcher=...".
set(CMAKE_C_CPPCHECK "${CMAKE_COMMAND};-E;true")
include(C-special-args.cmake)
