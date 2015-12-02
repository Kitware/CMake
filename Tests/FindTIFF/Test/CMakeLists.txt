cmake_minimum_required(VERSION 3.1)
project(TestFindTIFF C)
include(CTest)

# CMake does not actually provide FindTIFF publicly.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../Source/Modules)

find_package(TIFF REQUIRED)

add_executable(test_xercesc_tgt main.c)
target_link_libraries(test_xercesc_tgt TIFF::TIFF)
add_test(NAME test_xercesc_tgt COMMAND test_xercesc_tgt)

add_executable(test_xercesc_var main.c)
target_include_directories(test_xercesc_var PRIVATE ${TIFF_INCLUDE_DIRS})
target_link_libraries(test_xercesc_var PRIVATE ${TIFF_LIBRARIES})
add_test(NAME test_xercesc_var COMMAND test_xercesc_var)
