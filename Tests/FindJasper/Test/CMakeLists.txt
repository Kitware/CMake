cmake_minimum_required(VERSION 3.21)
project(TestFindJasper C)
include(CTest)

find_package(Jasper)

add_definitions(-DCMAKE_EXPECTED_JASPER_VERSION=${JASPER_VERSION_STRING})

add_executable(test_jasper_tgt main.c)
target_link_libraries(test_jasper_tgt Jasper::Jasper)
add_test(NAME test_jasper_tgt COMMAND test_jasper_tgt)

add_executable(test_jasper_var main.c)
target_include_directories(test_jasper_var PRIVATE ${JASPER_INCLUDE_DIRS})
target_link_libraries(test_jasper_var PRIVATE ${JASPER_LIBRARIES})
add_test(NAME test_jasper_var COMMAND test_jasper_var)
