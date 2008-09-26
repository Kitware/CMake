cmake_minimum_required(VERSION 2.4)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)
project(CURL C)

include_regular_expression("^.*$")

# Setup package meta-data
set(PACKAGE "curl")
set(VERSION "7.19.1")
set(PACKAGE_TARNAME "curl")
set(PACKAGE_BUGREPORT " ")
set(PACKAGE_NAME "curl")
set(PACKAGE_VERSION "-")
set(PACKAGE_STRING "curl-")
set(PACKAGE_BUGREPORT "a suitable curl mailing list => http://curl.haxx.se/mail/")
set(OS "${CMAKE_SYSTEM_NAME}")

# We need ansi c-flags, especially on HP
set(CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")
set(CMAKE_REQUIRED_FLAGS ${CMAKE_ANSI_CFLAGS})

# Disable warnings on Borland to avoid changing 3rd party code.
if(BORLAND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w-")
endif(BORLAND)

# If we are on AIX, do the _ALL_SOURCE magic
if(${CMAKE_SYSTEM_NAME} MATCHES AIX)
  set(_ALL_SOURCE 1)
endif(${CMAKE_SYSTEM_NAME} MATCHES AIX)

include_directories(
  ${CURL_BINARY_DIR}/include
  ${CURL_SOURCE_DIR}/include
  ${CURL_SOURCE_DIR}/lib
  ${CURL_BINARY_DIR}/lib)
if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
  add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005) 
add_definitions(-DHAVE_CONFIG_H
  -DCURL_STATICLIB)

add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(docs)

