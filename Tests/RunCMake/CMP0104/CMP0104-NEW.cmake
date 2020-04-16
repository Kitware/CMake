cmake_policy(SET CMP0104 NEW)
include(CMP0104-Common.cmake)

if(NOT CMAKE_CUDA_ARCHITECTURES)
  message(FATAL_ERROR "CMAKE_CUDA_ARCHITECTURES is empty with CMP0104 enabled.")
endif()
