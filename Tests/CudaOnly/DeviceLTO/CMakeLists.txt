cmake_minimum_required(VERSION 3.18)
project(DeviceLTO CUDA)

# Goal:
# Verify that we correctly compile with device LTO
# Verify that device LTO requirements are propagated to
# the final device link line

add_library(CUDA_dlto STATIC file1.cu file2.cu file3.cu)
add_executable(CudaOnlyDeviceLTO main.cu)

set(archs_to_test "${CMAKE_CUDA_ARCHITECTURES_ALL}")
if(CMAKE_CUDA_COMPILER_ID STREQUAL "NVIDIA")
  # Also test with at least one virtual architecture.
  list(POP_BACK CMAKE_CUDA_ARCHITECTURES_ALL_MAJOR latest_arch)
  list(APPEND archs_to_test ${latest_arch}-virtual)
endif()

set_target_properties(CUDA_dlto
                      PROPERTIES
                      CUDA_ARCHITECTURES "${archs_to_test}"
                      CUDA_SEPARABLE_COMPILATION ON
                      POSITION_INDEPENDENT_CODE ON)

set_target_properties(CudaOnlyDeviceLTO
                      PROPERTIES
                      CUDA_SEPARABLE_COMPILATION ON
                      CUDA_ARCHITECTURES "${archs_to_test}"
                      )

target_link_libraries(CudaOnlyDeviceLTO PRIVATE CUDA_dlto)

include(CheckIPOSupported)
check_ipo_supported(LANGUAGES CUDA RESULT ipo_supported)
if(ipo_supported)
  set_target_properties(CUDA_dlto
                        PROPERTIES
                        INTERPROCEDURAL_OPTIMIZATION ON)

  # When non-LTO variants (i.e. virtual) are built together with LTO ones the
  # linker warns about missing device LTO for the virtual architectures.
  # Ignore these warnings.
  target_link_options(CudaOnlyDeviceLTO PRIVATE "$<DEVICE_LINK:-w>")
endif()
