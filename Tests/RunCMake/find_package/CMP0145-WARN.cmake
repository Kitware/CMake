set(_FindDart_testing TRUE)
find_package(Dart MODULE)

if(NOT _FindDart_included)
  message(FATAL_ERROR "FindDart.cmake not included")
endif()
