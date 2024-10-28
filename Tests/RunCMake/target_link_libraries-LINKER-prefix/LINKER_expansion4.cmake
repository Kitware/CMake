
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
