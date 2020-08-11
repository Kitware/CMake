
enable_language(C)
enable_language(CXX)

if(BORLAND)
  set(pre -)
endif()

add_link_options ($<$<LINK_LANGUAGE:C>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${CMAKE_C_OUTPUT_EXTENSION}>
                  $<$<LINK_LANGUAGE:CXX>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${CMAKE_CXX_OUTPUT_EXTENSION}>)

add_library(LinkOptions_shared_c SHARED LinkOptionsLib.c)
add_library(LinkOptions_shared_cxx SHARED LinkOptionsLib.cxx)

add_library(LinkOptions_mod MODULE LinkOptionsLib.c)

add_executable(LinkOptions_exe LinkOptionsExe.c)
