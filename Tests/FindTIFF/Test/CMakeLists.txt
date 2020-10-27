cmake_minimum_required(VERSION 3.1)
project(TestFindTIFF)
include(CTest)

find_package(TIFF REQUIRED COMPONENTS CXX)

add_executable(test_tiff_tgt main.c)
target_link_libraries(test_tiff_tgt TIFF::TIFF)
add_test(NAME test_tiff_tgt COMMAND test_tiff_tgt)

add_executable(test_tiffxx_tgt main.cxx)
target_link_libraries(test_tiffxx_tgt TIFF::CXX)
add_test(NAME test_tiffxx_tgt COMMAND test_tiffxx_tgt)

add_executable(test_tiff_var main.c)
target_include_directories(test_tiff_var PRIVATE ${TIFF_INCLUDE_DIRS})
target_link_libraries(test_tiff_var PRIVATE ${TIFF_LIBRARIES})
add_test(NAME test_tiff_var COMMAND test_tiff_var)

add_executable(test_tiffxx_var main.cxx)
target_include_directories(test_tiffxx_var PRIVATE ${TIFF_INCLUDE_DIRS})
target_link_libraries(test_tiffxx_var PRIVATE ${TIFF_LIBRARIES})
add_test(NAME test_tiffxx_var COMMAND test_tiffxx_var)
