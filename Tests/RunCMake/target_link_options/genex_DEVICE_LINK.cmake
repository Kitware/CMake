
enable_language(C)

set (obj "${CMAKE_C_OUTPUT_EXTENSION}")
if(BORLAND)
  set(pre -)
endif()

add_library(LinkOptions_interface INTERFACE)
target_link_options (LinkOptions_interface INTERFACE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>
                                                     $<HOST_LINK:${pre}BADFLAG_NORMAL_LINK${obj}>)

add_library(LinkOptions_shared_interface SHARED LinkOptionsLib.c)
target_link_libraries (LinkOptions_shared_interface PRIVATE LinkOptions_interface)


add_library(LinkOptions_private SHARED LinkOptionsLib.c)
target_link_options (LinkOptions_private PRIVATE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>
                                                 $<HOST_LINK:${pre}BADFLAG_NORMAL_LINK${obj}>)
