project(ImportJavaNativeHeaders LANGUAGES NONE)

cmake_minimum_required (VERSION 3.19...3.20)
set(CMAKE_VERBOSE_MAKEFILE 1)

include(${NATIVE_HEADERS_IMPORT_DIR}/native.cmake)

if(NOT TARGET D1::D1-native)
  message(FATAL_ERROR "Target 'D1::D1-native' not found.")
endif()

get_property(incs TARGET D1::D1-native PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
if (NOT incs MATCHES "${NATIVE_HEADERS_IMPORT_DIR}/include")
  message(FATAL_ERROR "Target 'D1::D1-native', property 'INTERFACE_INCLUDE_DIRECTORIES' badly defined: ${incs}.")
endif()

if (NOT EXISTS "${NATIVE_HEADERS_IMPORT_DIR}/include/D.h")
  message(FATAL_ERROR "file '${NATIVE_HEADERS_IMPORT_DIR}/include/D.h' not found.")
endif()
