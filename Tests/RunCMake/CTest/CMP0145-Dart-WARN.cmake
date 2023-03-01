# Do not set CMP0145.
set(_FindDart_testing 1)
include(Dart)

if(NOT _FindDart_included)
  message(FATAL_ERROR "FindDart.cmake not included")
endif()
