if(NOT CMAKE_ANDROID_NDK_TOOLCHAIN_UNIFIED)
  return()
endif()

find_library(LIBDL dl)
if(NOT LIBDL)
  message(FATAL_ERROR "libdl not found.")
endif()

if(LIBDL MATCHES ".a$")
  message(FATAL_ERROR "found libdl.a")
endif()

find_program(CLANG clang)
if(NOT CLANG)
  message(FATAL_ERROR "clang not found")
endif()
