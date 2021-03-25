cmake_minimum_required(VERSION 3.10)
project(TestFindPostgreSQL C)
include(CTest)

find_package(PostgreSQL REQUIRED COMPONENTS Server)

add_definitions(-DCMAKE_EXPECTED_POSTGRESQL_VERSION="${PostgreSQL_VERSION_STRING}")

add_executable(test_tgt main.c)
target_link_libraries(test_tgt PostgreSQL::PostgreSQL)
add_test(NAME test_tgt COMMAND test_tgt)

add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${PostgreSQL_LIBRARIES})
add_test(NAME test_var COMMAND test_var)
