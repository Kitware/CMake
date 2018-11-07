cmake_minimum_required(VERSION 3.4)
project(TestFindSQLite3 C)
include(CTest)

find_package(SQLite3 REQUIRED)

add_definitions(-DCMAKE_EXPECTED_SQLite3_VERSION="${SQLite3_VERSION}")

add_executable(test_tgt main.c)
target_link_libraries(test_tgt SQLite::SQLite3)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${SQLite3_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${SQLite3_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
