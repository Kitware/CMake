set(_FindCABLE_testing TRUE)
find_package(CABLE MODULE)

if(NOT _FindCABLE_included)
  message(FATAL_ERROR "FindCABLE.cmake not included")
endif()
