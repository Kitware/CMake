cmake_minimum_required(VERSION 3.5)
project(CompilerIdOBJC OBJC)

foreach(v
    CMAKE_OBJC_COMPILER
    CMAKE_OBJC_COMPILER_ID
    CMAKE_OBJC_COMPILER_VERSION
    )
  if(${v})
    message(STATUS "${v}=[${${v}}]")
  else()
    message(SEND_ERROR "${v} not set!")
  endif()
endforeach()
