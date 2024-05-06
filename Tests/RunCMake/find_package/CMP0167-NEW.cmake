cmake_policy(SET CMP0167 NEW)
set(_FindBoost_testing TRUE)
find_package(Boost MODULE)

if(_FindBoost_included)
  message(FATAL_ERROR "FindBoost.cmake erroneously included")
endif()
