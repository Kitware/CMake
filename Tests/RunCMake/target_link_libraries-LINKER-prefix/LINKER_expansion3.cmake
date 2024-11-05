
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

# generate reference for LINKER flag
if (CMP0181 STREQUAL "NEW")
  if (CMAKE_C_LINKER_WRAPPER_FLAG)
    set(linker_flag ${CMAKE_C_LINKER_WRAPPER_FLAG})
    list(GET linker_flag -1 linker_space)
    if (linker_space STREQUAL " ")
      list(REMOVE_AT linker_flag -1)
    else()
      set(linker_space)
    endif()
    list (JOIN linker_flag " " linker_flag)
    if (CMAKE_C_LINKER_WRAPPER_FLAG_SEP)
      set(linker_sep "${CMAKE_C_LINKER_WRAPPER_FLAG_SEP}")

      string (APPEND  linker_flag "${linker_space}" "-foo${linker_sep}bar")
    else()
      set(linker_prefix "${linker_flag}${linker_space}")

      set (linker_flag "${linker_prefix}-foo ${linker_prefix}bar")
    endif()
  else()
    set(linker_flag "-foo bar")
  endif()

  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/LINKER.txt" "${linker_flag}")
endif()
