# Do not set CMP0191.
set(_FindCABLE_testing 1)
include(FindCABLE)

if(NOT _FindCABLE_included)
  message(FATAL_ERROR "FindCABLE.cmake not included")
endif()
