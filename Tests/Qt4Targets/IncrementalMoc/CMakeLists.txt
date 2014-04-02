
cmake_minimum_required(VERSION 2.8.12)
project(IncrementalMoc)

find_package(Qt4 REQUIRED)

qt4_generate_moc(foo.h moc_foo.cpp)

add_library(testlib foo.cpp moc_foo.cpp)
target_link_libraries(testlib Qt4::QtCore)
if (ADD_DEF)
  target_compile_definitions(testlib PRIVATE NEW_DEF)
endif()
