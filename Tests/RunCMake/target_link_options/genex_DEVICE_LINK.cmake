
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

if (CMake_TEST_CUDA)
  enable_language(CUDA)

  # Separable compilation is only supported on NVCC.
  if(NOT CMake_TEST_CUDA STREQUAL "Clang")
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
  endif()

  add_executable(LinkOptions_no_device LinkOptionsDevice.cu)
  target_link_options(LinkOptions_no_device PRIVATE $<DEVICE_LINK:${pre}BADFLAG_DEVICE_LINK${obj}>
                                                    $<HOST_LINK:${pre}BADFLAG_NORMAL_LINK${obj}>)
endif()
