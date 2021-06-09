cmake_minimum_required(VERSION 3.18)
project (EnableStandard HIP)

set(CMAKE_CXX_COMPILER ${CMAKE_HIP_COMPILER})
enable_language(CXX)

#Goal for this example:
#build hip sources that require C++11 to be enabled.

add_library(HIPStatic11 STATIC static.cxx)
set_source_files_properties(static.cxx PROPERTIES LANGUAGE HIP)

add_library(HIPDynamic11 SHARED shared.hip)

add_executable(HIPEnableStandard main.hip)
target_link_libraries(HIPEnableStandard PRIVATE HIPStatic11 HIPDynamic11)

target_compile_features(HIPDynamic11 PRIVATE cxx_std_11)
set_target_properties(HIPStatic11 PROPERTIES HIP_STANDARD 11)
set_target_properties(HIPStatic11 PROPERTIES HIP_STANDARD_REQUIRED TRUE)
