
enable_language(C)

cmake_policy(SET CMP0181 ${CMP0181})

# ensure command line is always displayed and do not use any response file
set(CMAKE_VERBOSE_MAKEFILE TRUE)

if (CMAKE_GENERATOR MATCHES "Borland|NMake")
  string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")
  string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")
endif()


set(CMAKE_C_CREATE_WIN32_EXE "${CMAKE_C_CREATE_WIN32_EXE} LINKER:-foo,bar")
add_executable (c_create_win32_exe WIN32 main.c)

set(CMAKE_C_CREATE_CONSOLE_EXE "${CMAKE_C_CREATE_CONSOLE_EXE} LINKER:-foo,bar")
add_executable(c_create_console_exe main.c)

if (CMP0181 STREQUAL "NEW")
  include(generate_linker_flag_reference.cmake)
endif()
