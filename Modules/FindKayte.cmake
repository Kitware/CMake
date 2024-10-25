# FindKayte.cmake - A custom CMake module to locate the Kayte Compiler

find_program(KAYTE_EXECUTABLE NAMES kaytec PATHS /usr/bin /usr/local/bin)

if (NOT KAYTE_EXECUTABLE)
    message(FATAL_ERROR "Kayte compiler not found. Install Kayte and try again.")
else ()
    message(STATUS "Found Kayte compiler: ${KAYTE_EXECUTABLE}")
    set(KAYTE_FOUND TRUE)
endif()

