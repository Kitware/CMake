cmake_minimum_required(VERSION 3.5)

project(pangoft2 C)

find_package(GTK2 COMPONENTS gtk REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(pangoft2 WIN32 main.c)
target_link_libraries(pangoft2 GTK2::pangoft2)
