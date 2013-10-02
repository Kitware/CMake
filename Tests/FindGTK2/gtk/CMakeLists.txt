cmake_minimum_required(VERSION 2.8)

project(gtk C)

find_package(GTK2 COMPONENTS gtk REQUIRED)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS GTK2::gtk
                       PROPERTIES IMPORTED_CONFIGURATIONS
                                  IMPORTED_LOCATION_RELEASE
                                  IMPORTED_LOCATION_DEBUG
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG
                                  INTERFACE_LINK_LIBRARIES
                                  INTERFACE_INCLUDE_DIRECTORIES
                                  INTERFACE_COMPILE_DEFINITIONS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(gtk WIN32 main.c)
target_link_libraries(gtk GTK2::gtk)

add_executable(gtk-all-libs WIN32 main.c)
target_link_libraries(gtk-all-libs ${GTK2_LIBRARIES})
target_include_directories(gtk-all-libs PRIVATE ${GTK2_INCLUDE_DIRS})
