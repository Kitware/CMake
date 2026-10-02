cmake_minimum_required(VERSION 3.10)
project(TestFindTCL C)
include(CTest)

find_package(TCL REQUIRED)

add_executable(test_tcl main.c)
target_include_directories(test_tcl PRIVATE ${TCL_INCLUDE_PATH})
target_link_libraries(test_tcl PRIVATE ${TCL_LIBRARY})
add_test(NAME test_tcl COMMAND test_tcl)

add_executable(test_tk main_tk.c)
target_include_directories(test_tk PRIVATE ${TK_INCLUDE_PATH} ${TCL_INCLUDE_PATH})
target_link_libraries(test_tk PRIVATE ${TK_LIBRARY} ${TCL_LIBRARY})
add_test(NAME test_tk COMMAND test_tk)
