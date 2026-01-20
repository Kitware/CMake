
enable_language(C)

set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

# CMAKE_STATIC_LINKER_FLAGS_<CONFIG> variant
set(CMAKE_STATIC_LINKER_FLAGS_RELEASE ${pre}BADFLAG_RELEASE${obj})
add_library(CMakeStaticLinkerFlags_config STATIC LinkOptionsLib.c)

# shared library do not use CMAKE_STATIC_LINKER_FLAGS_<CONFIG>
add_library(SharedCMakeStaticLinkerFlags_config SHARED LinkOptionsLib.c)
