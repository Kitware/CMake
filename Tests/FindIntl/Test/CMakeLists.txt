cmake_minimum_required(VERSION 3.10)
project(TestFindIntl CXX)
include(CTest)

find_package(Intl REQUIRED)

add_executable(test_intl_tgt main.cxx)
target_link_libraries(test_intl_tgt Intl::Intl)
add_test(NAME test_intl_tgt COMMAND test_intl_tgt)

add_executable(test_intl_var main.cxx)
target_include_directories(test_intl_var PRIVATE ${Intl_INCLUDE_DIRS})
target_link_libraries(test_intl_var PRIVATE ${Intl_LIBRARIES})
add_test(NAME test_intl_var COMMAND test_intl_var)
