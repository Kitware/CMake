cmake_minimum_required(VERSION 3.1)
project(TestFindGLEW LANGUAGES CXX)
include(CTest)

find_package(GLEW REQUIRED)

add_executable(test_glew_shared_tgt main.cpp)
target_link_libraries(test_glew_shared_tgt GLEW::GLEW)
add_test(NAME test_glew_shared_tgt COMMAND test_glew_shared_tgt)

add_executable(test_glew_generic_tgt main.cpp)
target_link_libraries(test_glew_generic_tgt GLEW::glew)
add_test(NAME test_glew_generic_tgt COMMAND test_glew_generic_tgt)

add_executable(test_glew_var main.cpp)
target_include_directories(test_glew_var PRIVATE ${GLEW_INCLUDE_DIRS})
target_link_libraries(test_glew_var PRIVATE ${GLEW_LIBRARIES})
add_test(NAME test_glew_var COMMAND test_glew_var)
