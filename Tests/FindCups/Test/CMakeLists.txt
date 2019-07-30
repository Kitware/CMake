cmake_minimum_required(VERSION 3.10)
project(TestFindCups C)
include(CTest)

find_package(Cups REQUIRED)

add_definitions(-DCMAKE_EXPECTED_CUPS_VERSION="${CUPS_VERSION_STRING}")

add_executable(test_tgt main.c)
target_link_libraries(test_tgt Cups::Cups)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${CUPS_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${CUPS_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
