cmake_minimum_required(VERSION 3.5)
project(TestFindOpenSP CXX)
include(CTest)

find_package(OpenSP REQUIRED)

add_definitions(-DSP_MULTI_BYTE="${OpenSP_MULTI_BYTE}")

add_executable(test_tgt main.cxx)
target_link_libraries(test_tgt OpenSP::OpenSP)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.cxx)
target_include_directories(test_var PRIVATE ${OpenSP_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${OpenSP_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
