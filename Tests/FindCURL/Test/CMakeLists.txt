cmake_minimum_required(VERSION 3.10)
project(TestFindCURL C)
include(CTest)

find_package(CURL REQUIRED COMPONENTS HTTP)

if(NOT CURL_VERSION MATCHES "^[0-9]+\\.[0-9]+(\\.|$)")
  message(FATAL_ERROR "CURL_VERSION not set")
endif()
if(NOT CURL_VERSION_STRING MATCHES "^[0-9]+\\.[0-9]+(\\.|$)")
  message(FATAL_ERROR "CURL_VERSION_STRING not set")
endif()

add_executable(test_tgt main.c)
target_link_libraries(test_tgt CURL::libcurl)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${CURL_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
