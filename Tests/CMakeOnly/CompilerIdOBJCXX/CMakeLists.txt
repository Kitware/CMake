cmake_minimum_required(VERSION 3.5)
project(CompilerIdOBJCXX OBJCXX)

foreach(v
    CMAKE_OBJCXX_COMPILER
    CMAKE_OBJCXX_COMPILER_ID
    CMAKE_OBJCXX_COMPILER_VERSION
    )
  if(${v})
    message(STATUS "${v}=[${${v}}]")
  else()
    message(SEND_ERROR "${v} not set!")
  endif()
endforeach()
