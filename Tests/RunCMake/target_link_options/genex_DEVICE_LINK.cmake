
enable_language(C)

set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)

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

if (CMake_TEST_CUDA)
  enable_language(CUDA)

  add_executable(LinkOptions_CMP0105_UNSET LinkOptionsDevice.cu)
  set_property(TARGET LinkOptions_CMP0105_UNSET PROPERTY CUDA_SEPARABLE_COMPILATION ON)
  target_link_options(LinkOptions_CMP0105_UNSET PRIVATE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>)

  cmake_policy(SET CMP0105 OLD)

  add_executable(LinkOptions_CMP0105_OLD LinkOptionsDevice.cu)
  set_property(TARGET LinkOptions_CMP0105_OLD PROPERTY CUDA_SEPARABLE_COMPILATION ON)
  target_link_options(LinkOptions_CMP0105_OLD PRIVATE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>)

  cmake_policy(SET CMP0105 NEW)

  add_executable(LinkOptions_CMP0105_NEW LinkOptionsDevice.cu)
  set_property(TARGET LinkOptions_CMP0105_NEW PROPERTY CUDA_SEPARABLE_COMPILATION ON)
  target_link_options(LinkOptions_CMP0105_NEW PRIVATE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>)

  add_executable(LinkOptions_device LinkOptionsDevice.cu)
  set_property(TARGET LinkOptions_device PROPERTY CUDA_SEPARABLE_COMPILATION ON)
  target_link_options(LinkOptions_device PRIVATE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>
                                                 $<HOST_LINK:${pre}BADFLAG_NORMAL_LINK${obj}>)

  add_executable(LinkOptions_host_link_options LinkOptionsDevice.cu)
  set_property(TARGET LinkOptions_host_link_options PROPERTY CUDA_SEPARABLE_COMPILATION ON)
  if(CMake_TEST_CUDA STREQUAL "NVIDIA")
    target_link_options(LinkOptions_host_link_options PRIVATE -Wl,OPT1 -Xlinker=OPT2 "SHELL:-Xlinker OPT3" "SHELL:LINKER:OPT4 LINKER:OPT5")
  elseif(CMake_TEST_CUDA STREQUAL "Clang")
    target_link_options(LinkOptions_host_link_options PRIVATE -Wl,OPT1 "SHELL:-Xlinker OPT2" "SHELL:LINKER:OPT3 LINKER:OPT4")
  endif()

  add_executable(LinkOptions_no_device LinkOptionsDevice.cu)
  target_link_options(LinkOptions_no_device PRIVATE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>
                                                    $<HOST_LINK:${pre}BADFLAG_NORMAL_LINK${obj}>)
endif()
