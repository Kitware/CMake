cmake_minimum_required(VERSION 3.10)
project(TestFindGDAL C)
include(CTest)

find_package(GDAL REQUIRED)

add_definitions(-DCMAKE_EXPECTED_GDAL_VERSION="${GDAL_VERSION}")

add_executable(test_tgt main.c)
target_link_libraries(test_tgt GDAL::GDAL)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${GDAL_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${GDAL_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
