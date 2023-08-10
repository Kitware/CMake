cmake_policy(SET CMP0146 NEW)
set(_FindCUDA_testing TRUE)
find_package(CUDA MODULE)

if(_FindCUDA_included)
  message(FATAL_ERROR "FindCUDA.cmake erroneously included")
endif()
