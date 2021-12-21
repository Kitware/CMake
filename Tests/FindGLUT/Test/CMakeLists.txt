cmake_minimum_required(VERSION 3.21)
project(TestFindGLUT C)
include(CTest)

find_package(GLUT REQUIRED)

add_executable(testglut_tgt main.c)
target_link_libraries(testglut_tgt GLUT::GLUT)
add_test(NAME testglut_tgt COMMAND testglut_tgt)

add_executable(testglut_var main.c)
target_include_directories(testglut_var PRIVATE ${GLUT_INCLUDE_DIRS})
target_link_libraries(testglut_var PRIVATE ${GLUT_LIBRARIES})
add_test(NAME testglut_var COMMAND testglut_var)

set_tests_properties(testglut_tgt testglut_var
    PROPERTIES WILL_FAIL true)
