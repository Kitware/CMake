
enable_language(C)
enable_language(CXX)

if(BORLAND)
  set(pre -)
endif()

add_library(LinkOptions_interface INTERFACE)
target_link_options (LinkOptions_interface INTERFACE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:${pre}BADFLAG_C_LANG_ID${CMAKE_C_OUTPUT_EXTENSION}>
                                                     $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${pre}BADFLAG_CXX_LANG_ID${CMAKE_CXX_OUTPUT_EXTENSION}>
                                                     $<$<LINK_LANG_AND_ID:C,BADID>:${pre}BADFLAG_C_BADID${CMAKE_C_OUTPUT_EXTENSION}>
                                                     $<$<LINK_LANG_AND_ID:CXX,BADID>:${pre}BADFLAG_CXX_BADID${CMAKE_CXX_OUTPUT_EXTENSION}>)

add_library(LinkOptions_shared_interface SHARED LinkOptionsLib.c)
target_link_libraries (LinkOptions_shared_interface PRIVATE LinkOptions_interface)

add_library(LinkOptions_shared_c SHARED LinkOptionsLib.c)
target_link_options (LinkOptions_shared_c PRIVATE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:${pre}BADFLAG_C_LANG_ID${CMAKE_C_OUTPUT_EXTENSION}>
                                                  $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${pre}BADFLAG_CXX_LANG_ID${CMAKE_CXX_OUTPUT_EXTENSION}>
                                                  $<$<LINK_LANG_AND_ID:C,BADID>:${pre}BADFLAG_C_BADID${CMAKE_C_OUTPUT_EXTENSION}>
                                                  $<$<LINK_LANG_AND_ID:CXX,BADID>:${pre}BADFLAG_CXX_BADID${CMAKE_CXX_OUTPUT_EXTENSION}>)

add_library(LinkOptions_shared_cxx SHARED LinkOptionsLib.c)
target_link_options (LinkOptions_shared_cxx PRIVATE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:${pre}BADFLAG_C_LANG_ID${CMAKE_C_OUTPUT_EXTENSION}>
                                                    $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${pre}BADFLAG_CXX_LANG_ID${CMAKE_CXX_OUTPUT_EXTENSION}>
                                                    $<$<LINK_LANG_AND_ID:C,BADID>:${pre}BADFLAG_C_BADID${CMAKE_C_OUTPUT_EXTENSION}>
                                                    $<$<LINK_LANG_AND_ID:CXX,BADID>:${pre}BADFLAG_CXX_BADID${CMAKE_CXX_OUTPUT_EXTENSION}>)
set_property (TARGET LinkOptions_shared_cxx PROPERTY LINKER_LANGUAGE CXX)

add_library(LinkOptions_mod MODULE LinkOptionsLib.c)
target_link_options (LinkOptions_mod PRIVATE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:${pre}BADFLAG_C_LANG_ID${CMAKE_C_OUTPUT_EXTENSION}>
                                             $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${pre}BADFLAG_CXX_LANG_ID${CMAKE_CXX_OUTPUT_EXTENSION}>
                                             $<$<LINK_LANG_AND_ID:C,BADID>:${pre}BADFLAG_C_BADID${CMAKE_C_OUTPUT_EXTENSION}>
                                             $<$<LINK_LANG_AND_ID:CXX,BADID>:${pre}BADFLAG_CXX_BADID${CMAKE_CXX_OUTPUT_EXTENSION}>)

add_executable(LinkOptions_exe LinkOptionsExe.c)
target_link_options (LinkOptions_exe PRIVATE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:${pre}BADFLAG_C_LANG_ID${CMAKE_C_OUTPUT_EXTENSION}>
                                             $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${pre}BADFLAG_CXX_LANG_ID${CMAKE_CXX_OUTPUT_EXTENSION}>
                                             $<$<LINK_LANG_AND_ID:C,BADID>:${pre}BADFLAG_C_BADID${CMAKE_C_OUTPUT_EXTENSION}>
                                             $<$<LINK_LANG_AND_ID:CXX,BADID>:${pre}BADFLAG_CXX_BADID${CMAKE_CXX_OUTPUT_EXTENSION}>)
