# FindFreePascal.cmake - A custom CMake module to locate the Free Pascal Compiler

find_program(FPC_EXECUTABLE NAMES fpc PATHS /usr/bin /usr/local/bin)

if (NOT FPC_EXECUTABLE)
    message(FATAL_ERROR "Free Pascal Compiler not found. Install FPC and try again.")
else ()
    message(STATUS "Found Free Pascal Compiler: ${FPC_EXECUTABLE}")
    set(FPC_FOUND TRUE)
endif()

# Add FPC as a language in the CMake project.
enable_language(Pascal)

