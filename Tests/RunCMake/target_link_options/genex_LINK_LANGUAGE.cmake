
enable_language(C)
enable_language(CXX)

set (obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

add_library(LinkOptions_interface INTERFACE)
target_link_options (LinkOptions_interface INTERFACE $<$<LINK_LANGUAGE:C>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>
                                                     $<$<LINK_LANGUAGE:CXX>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>)

add_library(LinkOptions_shared_interface SHARED LinkOptionsLib.c)
target_link_libraries (LinkOptions_shared_interface PRIVATE LinkOptions_interface)

add_library(LinkOptions_shared_c SHARED LinkOptionsLib.c)
target_link_options (LinkOptions_shared_c PRIVATE $<$<LINK_LANGUAGE:C>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>
                                                  $<$<LINK_LANGUAGE:CXX>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>)

add_library(LinkOptions_shared_cxx SHARED LinkOptionsLib.c)
target_link_options (LinkOptions_shared_cxx PRIVATE $<$<LINK_LANGUAGE:C>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>
                                                    $<$<LINK_LANGUAGE:CXX>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>)
set_property (TARGET LinkOptions_shared_cxx PROPERTY LINKER_LANGUAGE CXX)

add_library(LinkOptions_mod MODULE LinkOptionsLib.c)
target_link_options (LinkOptions_mod PRIVATE $<$<LINK_LANGUAGE:C>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>
                                             $<$<LINK_LANGUAGE:CXX>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>)

add_executable(LinkOptions_exe LinkOptionsExe.c)
target_link_options (LinkOptions_exe PRIVATE $<$<LINK_LANGUAGE:C>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>
                                             $<$<LINK_LANGUAGE:CXX>:${pre}BADFLAG_$<LINK_LANGUAGE>_LANG${obj}>)
