cmake_minimum_required(VERSION 2.8)

project(gtk)

find_package(GTK2 COMPONENTS gtk REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(gtk WIN32 main.c)
target_link_libraries(gtk GTK2::gtk)
