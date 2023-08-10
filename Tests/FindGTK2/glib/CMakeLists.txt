cmake_minimum_required(VERSION 3.5)

project(glib C)

find_package(GTK2 COMPONENTS gtk REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(glib WIN32 main.c)
target_link_libraries(glib GTK2::glib)
