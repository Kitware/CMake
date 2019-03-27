cmake_minimum_required(VERSION 3.10)
project(TestFindFontconfig C)
include(CTest)

find_package(Fontconfig REQUIRED)

add_definitions(-DCMAKE_EXPECTED_FONTCONFIG_VERSION="${Fontconfig_VERSION}")

add_executable(test_tgt main.c)
target_link_libraries(test_tgt Fontconfig::Fontconfig)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${Fontconfig_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${Fontconfig_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
