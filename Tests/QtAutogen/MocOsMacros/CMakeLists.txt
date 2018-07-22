cmake_minimum_required(VERSION 3.11)
project(MocOsMacros)
include("../AutogenTest.cmake")

# Tests if moc processes the macro Q_OS_MAC

set(CMAKE_AUTOMOC True)

add_executable(mocOsMacros
  main.cpp
  TestClass.cpp
  TestClass.hpp
)
target_link_libraries(mocOsMacros PRIVATE ${QT_QTCORE_TARGET})

message ( "CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}" )
