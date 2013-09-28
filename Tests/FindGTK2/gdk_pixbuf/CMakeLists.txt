cmake_minimum_required(VERSION 2.8)

project(gdk_pixbuf)

find_package(GTK2 COMPONENTS gtk REQUIRED)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS GTK2::gdk_pixbuf
                       PROPERTIES IMPORTED_CONFIGURATIONS
                                  IMPORTED_LOCATION_RELEASE
                                  IMPORTED_LOCATION_DEBUG
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG
                                  INTERFACE_LINK_LIBRARIES
                                  INTERFACE_INCLUDE_DIRECTORIES
                                  INTERFACE_COMPILE_DEFINITIONS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(gdk_pixbuf WIN32 main.c)
target_link_libraries(gdk_pixbuf GTK2::gdk_pixbuf)
