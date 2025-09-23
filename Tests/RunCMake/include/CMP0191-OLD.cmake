cmake_policy(SET CMP0191 OLD)
set(_FindCABLE_testing 1)
include(FindCABLE)

if(NOT _FindCABLE_included)
  message(FATAL_ERROR "FindCABLE.cmake not included")
endif()
