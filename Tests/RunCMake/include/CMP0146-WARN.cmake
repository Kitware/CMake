# Do not set CMP0146.
set(_FindCUDA_testing 1)
include(FindCUDA)

if(NOT _FindCUDA_included)
  message(FATAL_ERROR "FindCUDA.cmake not included")
endif()
