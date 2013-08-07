cmake_minimum_required(VERSION 2.8)

project(GTK2GtkMM)

find_package(GTK2 COMPONENTS gtk gtkmm REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(GTK2GtkMM WIN32 main.cpp helloworld.cpp helloworld.h)
target_link_libraries(GTK2GtkMM GTK2::gtkmm)
