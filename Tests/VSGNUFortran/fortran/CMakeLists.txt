cmake_minimum_required(VERSION 2.8)
project(FortranHello Fortran C)

include(FortranCInterface)
FortranCInterface_HEADER(HelloWorldFCMangle.h
                         MACRO_NAMESPACE "FC_"
                         SYMBOL_NAMESPACE "FC_"
                         SYMBOLS hello world)

add_library(hello SHARED hello.f)
add_library(world SHARED world.f)
target_link_libraries(hello world)
if(CMAKE_Fortran_COMPILER_ID MATCHES SunPro)
  target_link_libraries(hello fsu)
  if(CMAKE_Fortran_PLATFORM_ID MATCHES SunOS)
    target_link_libraries(hello fio sunmath)
  endif()
endif()
