cmake_minimum_required (VERSION 2.6)
PROJECT(hello Java)
SET(CMAKE_VERBOSE_MAKEFILE 1)
  
IF("${hello_SOURCE_DIR}" STREQUAL "${hello_BINARY_DIR}")
  MESSAGE("In Source, building Java using ADD_CUSTOM_COMMAND()")
  SET(OLD_CUSTOM_COMMAND_WAY 1)
ELSE("${hello_SOURCE_DIR}" STREQUAL "${hello_BINARY_DIR}")
  MESSAGE("Out of source, using built-in Java support")
  SET(OLD_CUSTOM_COMMAND_WAY )
ENDIF("${hello_SOURCE_DIR}" STREQUAL "${hello_BINARY_DIR}")

IF(NOT OLD_CUSTOM_COMMAND_WAY)

  INCLUDE_DIRECTORIES(${hello_SOURCE_DIR}
    ${hello_BINARY_DIR} )
  ADD_LIBRARY(hello A.java HelloWorld.java)

ELSE(NOT OLD_CUSTOM_COMMAND_WAY)

  ADD_CUSTOM_COMMAND(
    OUTPUT ${hello_BINARY_DIR}/A.class
    MAIN_DEPENDENCY ${hello_SOURCE_DIR}/A.java
    COMMAND ${CMAKE_Java_COMPILER} ARGS -classpath . -d ${hello_BINARY_DIR} ${hello_SOURCE_DIR}/A.java)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${hello_BINARY_DIR}/HelloWorld.class
    MAIN_DEPENDENCY ${hello_SOURCE_DIR}/HelloWorld.java
    DEPENDS ${hello_BINARY_DIR}/A.class
    COMMAND ${CMAKE_Java_COMPILER} ARGS -classpath . -d ${hello_BINARY_DIR} ${hello_SOURCE_DIR}/HelloWorld.java)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${hello_BINARY_DIR}/hello.jar
    DEPENDS ${hello_BINARY_DIR}/A.class ${hello_BINARY_DIR}/HelloWorld.class
    COMMAND ${CMAKE_COMMAND}
    ARGS -E chdir ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_Java_ARCHIVE}
    -cvf hello.jar *.class)
  
  ADD_CUSTOM_TARGET(hello_jar ALL 
    DEPENDS ${hello_BINARY_DIR}/hello.jar)

ENDIF(NOT OLD_CUSTOM_COMMAND_WAY)

