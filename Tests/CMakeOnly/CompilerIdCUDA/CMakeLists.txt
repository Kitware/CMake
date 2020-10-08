cmake_minimum_required(VERSION 3.18)
project(CompilerIdCUDA CUDA)

foreach(v
    CMAKE_CUDA_COMPILER
    CMAKE_CUDA_COMPILER_ID
    CMAKE_CUDA_COMPILER_VERSION
    )
  if(${v})
    message(STATUS "${v}=[${${v}}]")
  else()
    message(SEND_ERROR "${v} not set!")
  endif()
endforeach()
