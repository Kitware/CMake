
enable_language(C)
enable_language(CXX)

if(BORLAND)
  set(pre -)
endif()

add_link_options ($<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:${pre}BADFLAG_C_LANG_ID${CMAKE_C_OUTPUT_EXTENSION}>
                  $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${pre}BADFLAG_CXX_LANG_ID${CMAKE_CXX_OUTPUT_EXTENSION}>
                  $<$<LINK_LANG_AND_ID:C,BADID>:${pre}BADFLAG_C_BADID${CMAKE_C_OUTPUT_EXTENSION}>
                  $<$<LINK_LANG_AND_ID:CXX,BADID>:${pre}BADFLAG_CXX_BADID${CMAKE_CXX_OUTPUT_EXTENSION}>)

add_library(LinkOptions_shared_c SHARED LinkOptionsLib.c)
add_library(LinkOptions_shared_cxx SHARED LinkOptionsLib.cxx)

add_library(LinkOptions_mod MODULE LinkOptionsLib.c)

add_executable(LinkOptions_exe LinkOptionsExe.c)
