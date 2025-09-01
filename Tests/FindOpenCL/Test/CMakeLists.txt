cmake_minimum_required(VERSION 3.10)
project(TestFindOpenCL C)
include(CTest)

find_package(OpenCL REQUIRED)

add_compile_definitions(
  CL_TARGET_OPENCL_VERSION=${OpenCL_VERSION_MAJOR}${OpenCL_VERSION_MINOR}0
)

add_executable(test_tgt main.c)
target_link_libraries(test_tgt OpenCL::OpenCL)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${OpenCL_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${OpenCL_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
