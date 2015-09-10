cmake_policy(SET CMP0066 NEW)
enable_language(C)

add_library(cmp0066test test.c)
if (DEFINED cmp0066test_LIB_DEPENDS)
  message(FATAL_ERROR "_LIB_DEPENDS variable is not supposed to be defined")
endif ()
