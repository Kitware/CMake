cmake_minimum_required(VERSION 3.18)
project(HIPArchitecture HIP)

# Make sure CMake doesn't pass architectures if HIP_ARCHITECTURES is OFF.
string(APPEND CMAKE_HIP_FLAGS " --offload-arch=gfx908")

add_executable(HIPOnlyArchitectureOff main.hip)
set_property(TARGET HIPOnlyArchitectureOff PROPERTY HIP_ARCHITECTURES OFF)
