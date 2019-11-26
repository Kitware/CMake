project(hello Java)

cmake_minimum_required (VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE 1)

include(CTest)

find_package(Java COMPONENTS Development)
include (UseJava)

add_jar(hello A.java HelloWorld.java)

# use listing file to specify sources
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/java_fileslist "A.java\nHelloWorld.java\n")
add_jar(hello2 @${CMAKE_CURRENT_BINARY_DIR}/java_fileslist)

# use listing file to specify sources and specify output directory (issue #17316)
add_jar(hello3 @${CMAKE_CURRENT_BINARY_DIR}/java_fileslist OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/hello3")

add_test (NAME Java.Jar
          COMMAND "${Java_JAVA_EXECUTABLE}" -classpath hello.jar HelloWorld)
add_test (NAME Java.JarSourceList
          COMMAND "${Java_JAVA_EXECUTABLE}" -classpath hello2.jar HelloWorld)
add_test (NAME Java.JarSourceListAndOutput
          COMMAND "${Java_JAVA_EXECUTABLE}" -classpath "${CMAKE_CURRENT_BINARY_DIR}/hello3/hello3.jar" HelloWorld)
