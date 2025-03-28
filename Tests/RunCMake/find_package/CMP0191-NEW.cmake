cmake_policy(SET CMP0191 NEW)
set(_FindCABLE_testing TRUE)
find_package(CABLE MODULE)

if(_FindCABLE_included)
  message(FATAL_ERROR "FindCABLE.cmake erroneously included")
endif()
