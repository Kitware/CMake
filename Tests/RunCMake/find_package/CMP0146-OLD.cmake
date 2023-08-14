cmake_policy(SET CMP0146 OLD)
set(_FindCUDA_testing TRUE)
find_package(CUDA MODULE)

if(NOT _FindCUDA_included)
  message(FATAL_ERROR "FindCUDA.cmake not included")
endif()
