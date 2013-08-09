cmake_minimum_required(VERSION 2.8)

project(gtkmm)

find_package(GTK2 COMPONENTS gtk gtkmm REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(gtkmm WIN32 main.cpp helloworld.cpp helloworld.h)
target_link_libraries(gtkmm GTK2::gtkmm)
