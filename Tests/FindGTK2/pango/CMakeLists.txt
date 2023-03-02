cmake_minimum_required(VERSION 3.5)

project(pango C)

find_package(GTK2 COMPONENTS gtk REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(pango WIN32 main.c)
target_link_libraries(pango GTK2::pango)
