
enable_language(C)

set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

add_library(StaticLinkFlags STATIC LinkOptionsLib.c)
set_property(TARGET StaticLinkFlags PROPERTY STATIC_LIBRARY_FLAGS ${pre}BADFLAG${obj})

# STATIC_LIBRARY_FLAGS_<CONFIG> variant
add_library(StaticLinkFlags_config STATIC LinkOptionsLib.c)
set_property(TARGET StaticLinkFlags_config PROPERTY STATIC_LIBRARY_FLAGS_RELEASE ${pre}BADFLAG_RELEASE${obj})

# shared library do not use property STATIC_LIBRARY_FLAGS
add_library(SharedLinkFlags SHARED LinkOptionsLib.c)
set_property(TARGET SharedLinkFlags PROPERTY STATIC_LIBRARY_FLAGS ${pre}BADFLAG${obj})
