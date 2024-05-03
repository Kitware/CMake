cmake_policy(SET CMP0167 OLD)
set(_FindBoost_testing 1)
include(FindBoost)

if(NOT _FindBoost_included)
  message(FATAL_ERROR "FindBoost.cmake not included")
endif()
