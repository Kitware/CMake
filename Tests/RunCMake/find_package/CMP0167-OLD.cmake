cmake_policy(SET CMP0167 OLD)
set(_FindBoost_testing TRUE)
find_package(Boost MODULE)

if(NOT _FindBoost_included)
  message(FATAL_ERROR "FindBoost.cmake not included")
endif()
