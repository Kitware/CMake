cmake_minimum_required(VERSION 3.26)
project(TestFindwxWidgets CXX)
include(CTest)

find_package(wxWidgets REQUIRED)

add_executable(test_tgt main.cpp)
target_link_libraries(test_tgt wxWidgets::wxWidgets)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.cpp)
target_link_libraries(test_var PRIVATE ${wxWidgets_LIBRARIES})
target_link_directories(test_var PRIVATE ${wxWidgets_LIBRARY_DIRS})
target_include_directories(test_var PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_compile_options(test_var PRIVATE ${wxWidgets_CONFIG_OPTIONS})
target_compile_definitions(test_var PRIVATE ${wxWidgets_DEFINITIONS})
add_test(NAME test_var COMMAND test_var)
