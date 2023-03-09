cmake_minimum_required(VERSION 3.5)
project(TestFindGIF C)
include(CTest)

find_package(GIF REQUIRED)

add_definitions(-DCMAKE_EXPECTED_GIF_VERSION="${GIF_VERSION}")

add_executable(test_tgt main.c)
target_link_libraries(test_tgt GIF::GIF)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${GIF_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${GIF_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
