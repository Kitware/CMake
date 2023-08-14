cmake_policy(SET CMP0145 NEW)
set(_FindDart_testing TRUE)
find_package(Dart MODULE)

if(_FindDart_included)
  message(FATAL_ERROR "FindDart.cmake erroneously included")
endif()
