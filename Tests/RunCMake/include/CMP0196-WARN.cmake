# Do not set CMP0196.
set(_CMakeDetermineVSServicePack_testing 1)
include(CMakeDetermineVSServicePack)

if(NOT _CMakeDetermineVSServicePack_included)
  message(FATAL_ERROR "CMakeDetermineVSServicePack.cmake not included")
endif()
