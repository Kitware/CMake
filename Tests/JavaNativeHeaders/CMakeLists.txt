project(helloJavaNativeHeaders Java CXX)

cmake_minimum_required (VERSION 2.8.12)
set(CMAKE_VERBOSE_MAKEFILE 1)

include (CTest)

find_package(Java COMPONENTS Development)
include (UseJava)

# JNI support
find_package(JNI)

add_jar(B1 D.java GENERATE_NATIVE_HEADERS D1-native DESTINATION INSTALL include)
add_jar(E1 E.java GENERATE_NATIVE_HEADERS E1-native)

add_jar(hello4 HelloWorld3.java)

add_library(D SHARED D.cpp E.cpp)
target_link_libraries (D PRIVATE D1-native E1-native)

install(TARGETS D1-native EXPORT native)
install(DIRECTORY "$<TARGET_PROPERTY:D1-native,NATIVE_HEADERS_DIRECTORY>/" DESTINATION include)
install(EXPORT native DESTINATION "${CMAKE_INSTALL_PREFIX}" NAMESPACE D1::)


add_test (NAME Java.NativeHeaders
          COMMAND "${Java_JAVA_EXECUTABLE}" -Djava.library.path=$<TARGET_FILE_DIR:D> -classpath hello4.jar HelloWorld3)

add_test (NAME Java.ImportNativeHeaders
          COMMAND "${CMAKE_COMMAND}" "-DNATIVE_HEADERS_IMPORT_DIR=${CMAKE_INSTALL_PREFIX}" -S "${CMAKE_CURRENT_SOURCE_DIR}/Import" -B "${CMAKE_CURRENT_BINARY_DIR}/Import")
