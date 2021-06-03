project(helloJavah Java CXX)

cmake_minimum_required (VERSION 2.8.12)
set(CMAKE_VERBOSE_MAKEFILE 1)

include(CTest)

find_package(Java COMPONENTS Development)
include (UseJava)

# JNI support
find_package(JNI)

add_jar(B1 B.java)
add_jar(C1 C.java)
create_javah(TARGET B_javah CLASSES B C CLASSPATH B1 C1)

add_jar(hello3 HelloWorld2.java)

add_library(B SHARED B.cpp C.cpp)
add_dependencies(B B_javah)

target_include_directories(B PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
                                     ${JAVA_INCLUDE_PATH}
                                     ${JAVA_INCLUDE_PATH2})

add_test (NAME Java.Javah
          COMMAND "${Java_JAVA_EXECUTABLE}" -Djava.library.path=$<TARGET_FILE_DIR:B> -classpath hello3.jar HelloWorld2)
