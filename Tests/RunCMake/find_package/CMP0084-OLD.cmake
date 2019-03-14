cmake_policy(SET CMP0084 OLD)
set(_findqt_testing TRUE)
find_package(Qt MODULE)

if(NOT _findqt_included)
  message(FATAL_ERROR "FindQt.cmake not included")
endif()
