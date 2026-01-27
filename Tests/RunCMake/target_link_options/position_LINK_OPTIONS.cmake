
enable_language(C)

set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)

# shared configuration
string(APPEND CMAKE_SHARED_LINKER_FLAGS " ${pre}BADFLAG_GLOBAL${obj}")
add_library(LinkOptions_shared SHARED LinkOptionsLib.c)
target_link_options(LinkOptions_shared PRIVATE ${pre}BADFLAG_PRIVATE${obj})

# module configuration
string(APPEND CMAKE_MODULE_LINKER_FLAGS " ${pre}BADFLAG_GLOBAL${obj}")
add_library(LinkOptions_module MODULE LinkOptionsLib.c)
target_link_options(LinkOptions_module PRIVATE ${pre}BADFLAG_PRIVATE${obj})

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # framework configuration
  string(APPEND CMAKE_MACOSX_FRAMEWORK_LINKER_FLAGS " ${pre}BADFLAG_GLOBAL${obj}")
  add_library(LinkOptions_framework SHARED LinkOptionsLib.c)
  set_property(TARGET LinkOptions_framework PROPERTY FRAMEWORK TRUE)
  target_link_options(LinkOptions_framework PRIVATE ${pre}BADFLAG_PRIVATE${obj})
endif()
