cmake_minimum_required(VERSION 3.12)
project(TestFindLibArchive C)
include(CTest)

find_package(LibArchive REQUIRED)

add_executable(test_libarchive_tgt main.c)
target_link_libraries(test_libarchive_tgt LibArchive::LibArchive)
add_test(NAME test_libarchive_tgt COMMAND test_libarchive_tgt)

add_executable(test_libarchive_var main.c)
target_include_directories(test_libarchive_var PRIVATE ${LibArchive_INCLUDE_DIRS})
target_link_libraries(test_libarchive_var PRIVATE ${LibArchive_LIBRARIES})
add_test(NAME test_libarchive_var COMMAND test_libarchive_var)
