cmake_policy(SET CMP0196 OLD)
set(_CMakeDetermineVSServicePack_testing 1)
include(CMakeDetermineVSServicePack)

if(NOT _CMakeDetermineVSServicePack_included)
  message(FATAL_ERROR "CMakeDetermineVSServicePack.cmake not included")
endif()
