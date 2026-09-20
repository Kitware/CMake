include(common.cmake)
enable_language(C)

# Launchers that evaluate to nothing must not disturb the wrappers.
set(CMAKE_C_COMPILER_LAUNCHER "$<0:unused>")
set(CMAKE_C_LINKER_LAUNCHER "$<0:unused>")
example_exe(main.c)
