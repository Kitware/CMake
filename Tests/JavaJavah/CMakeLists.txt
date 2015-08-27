project(helloJavah Java CXX)

cmake_minimum_required (VERSION 2.6)
set(CMAKE_VERBOSE_MAKEFILE 1)

find_package(Java COMPONENTS Development)
include (UseJava)

# JNI support
find_package(JNI)

add_jar(hello3 B.java HelloWorld2.java)
create_javah(TARGET B_javah CLASSES B CLASSPATH hello3)

add_library(B SHARED B.cpp)
add_dependencies(B B_javah)

target_include_directories(B PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
                                     ${JAVA_INCLUDE_PATH}
                                     ${JAVA_INCLUDE_PATH2})
