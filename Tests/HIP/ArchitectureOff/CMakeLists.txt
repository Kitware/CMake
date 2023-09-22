cmake_minimum_required(VERSION 3.18)
project(HIPArchitecture HIP)

# Make sure CMake doesn't pass architectures if HIP_ARCHITECTURES is OFF.
set(CMAKE_HIP_ARCHITECTURES OFF)

# Pass our own architecture flags instead.
if(CMAKE_HIP_PLATFORM STREQUAL "amd")
  string(APPEND CMAKE_HIP_FLAGS " --offload-arch=gfx908")
elseif(CMAKE_HIP_PLATFORM STREQUAL "nvidia")
  string(APPEND CMAKE_HIP_FLAGS " -arch=sm_52")
endif()

add_executable(HIPOnlyArchitectureOff main.hip)
get_property(hip_archs TARGET HIPOnlyArchitectureOff PROPERTY HIP_ARCHITECTURES)
if(NOT hip_archs STREQUAL "OFF")
  message(FATAL_ERROR "CMAKE_HIP_ARCHITECTURES did not initialize HIP_ARCHITECTURES")
endif()
