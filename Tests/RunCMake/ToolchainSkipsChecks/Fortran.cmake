enable_language(Fortran)

# See #27894
if (NOT CMAKE_SIZEOF_VOID_P)
  message(FATAL_ERROR
    "`CMAKE_SIZEOF_VOID_P` not initialized from toolchain data")
endif ()
if (CMAKE_Fortran_PLATFORM_ABI AND NOT CMAKE_INTERNAL_PLATFORM_ABI)
  message(FATAL_ERROR
    "`CMAKE_INTERNAL_PLATFORM_ABI` not initialized from toolchain data")
endif ()
if (CMAKE_Fortran_LIBRARY_ARCHITECTURE AND NOT CMAKE_LIBRARY_ARCHITECTURE)
  message(FATAL_ERROR
    "`CMAKE_LIBRARY_ARCHITECTURE` not initialized from toolchain data")
endif ()
