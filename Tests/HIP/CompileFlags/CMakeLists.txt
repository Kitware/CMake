cmake_minimum_required(VERSION 3.18)
project(CompileFlags HIP)

add_executable(HIPOnlyCompileFlags main.hip)

if(CMAKE_HIP_PLATFORM STREQUAL "amd")
  set(hip_archs gfx803)
elseif(CMAKE_HIP_PLATFORM STREQUAL "nvidia")
  set(hip_archs 52)
endif()
set_property(TARGET HIPOnlyCompileFlags PROPERTY HIP_ARCHITECTURES ${hip_archs})

target_compile_options(HIPOnlyCompileFlags PRIVATE -DALWAYS_DEFINE)
