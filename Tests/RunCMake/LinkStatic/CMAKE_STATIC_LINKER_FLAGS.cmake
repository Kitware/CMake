
enable_language(C)

set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

set(CMAKE_STATIC_LINKER_FLAGS ${pre}BADFLAG${obj})
add_library(CMakeStaticLinkerFlags STATIC LinkOptionsLib.c)

# shared library do not use CMAKE_STATIC_LINKER_FLAGS
add_library(SharedCMakeStaticLinkerFlags SHARED LinkOptionsLib.c)
