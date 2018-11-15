cmake_policy(SET CMP0084 NEW)
set(_findqt_testing TRUE)
find_package(Qt MODULE)

if(_findqt_included)
  message(FATAL_ERROR "FindQt.cmake erroneously included")
endif()
