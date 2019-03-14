cmake_minimum_required(VERSION 3.10)
project(TestFindLibinput C)
include(CTest)

find_package(Libinput REQUIRED)

add_executable(test_tgt main.c)
target_link_libraries(test_tgt Libinput::Libinput)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${Libinput_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${Libinput_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
