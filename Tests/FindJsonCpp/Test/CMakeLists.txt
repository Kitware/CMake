cmake_minimum_required(VERSION 3.1)
project(TestFindJsonCpp CXX)
include(CTest)

find_package(JsonCpp REQUIRED)

add_executable(test_jsoncpp_tgt main.cxx)
target_link_libraries(test_jsoncpp_tgt JsonCpp::JsonCpp)
add_test(NAME test_jsoncpp_tgt COMMAND test_jsoncpp_tgt)

add_executable(test_jsoncpp_var main.cxx)
target_include_directories(test_jsoncpp_var PRIVATE ${JsonCpp_INCLUDE_DIRS})
target_link_libraries(test_jsoncpp_var PRIVATE ${JsonCpp_LIBRARIES})
add_test(NAME test_jsoncpp_var COMMAND test_jsoncpp_var)
