
enable_language(C)

set(obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

add_link_options ($<$<LINK_LANGUAGE:C>:${pre}BADFLAG_LANG_C${obj}>
                  $<$<LINK_LANGUAGE:CXX>:${pre}BADFLAG_LANG_CXX${obj}>)

add_library(LinkOptions_shared SHARED LinkOptionsLib.c)

add_library(LinkOptions_mod MODULE LinkOptionsLib.c)

add_executable(LinkOptions_exe LinkOptionsExe.c)
