enable_language(C)

if(NOT CMAKE_C_COMPILER_LINKER OR NOT CMAKE_C_COMPILER_LINKER_ID)
  message(FATAL_ERROR "Failed to determine Linker.")
endif()
