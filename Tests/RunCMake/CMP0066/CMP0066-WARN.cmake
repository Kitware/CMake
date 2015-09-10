cmake_policy(SET CMP0066 OLD)
enable_language(C)

add_library(cmp0066test test.c)
if (NOT DEFINED cmp0066test_LIB_DEPENDS)
  message(FATAL_ERROR "_LIB_DEPENDS variable is supposed to be defined")
endif ()
