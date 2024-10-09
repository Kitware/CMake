# Do not set CMP0167.
set(_FindBoost_testing 1)
include(FindBoost)

if(NOT _FindBoost_included)
  message(FATAL_ERROR "FindBoost.cmake not included")
endif()
