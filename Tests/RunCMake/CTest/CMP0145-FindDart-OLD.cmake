cmake_policy(SET CMP0145 OLD)
set(_FindDart_testing 1)
include(FindDart)

if(NOT _FindDart_included)
  message(FATAL_ERROR "FindDart.cmake not included")
endif()
