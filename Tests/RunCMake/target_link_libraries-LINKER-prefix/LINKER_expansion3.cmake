
enable_language(C)

cmake_policy(SET CMP0181 ${CMP0181})

# ensure command line is always displayed and do not use any response file
set(CMAKE_VERBOSE_MAKEFILE TRUE)

if (CMAKE_GENERATOR MATCHES "Borland|NMake")
  string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")
  string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")

  string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")
  string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")

  string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_MODULE "${CMAKE_C_CREATE_SHARED_MODULE}")
  string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_MODULE "${CMAKE_C_CREATE_SHARED_MODULE}")
endif()


set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} LINKER:-foo,bar")
add_executable(c_exe_create_link_flags main.c)

set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} LINKER:-foo,bar")
add_library(c_shared_create_link_flags SHARED LinkOptionsLib.c)

set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} LINKER:-foo,bar")
add_library(c_module_create_link_flags MODULE LinkOptionsLib.c)

if (CMP0181 STREQUAL "NEW")
  include(generate_linker_flag_reference.cmake)
endif()
