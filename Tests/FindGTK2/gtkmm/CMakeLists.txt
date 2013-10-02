cmake_minimum_required(VERSION 2.8)

project(gtkmm CXX)

message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")
set(CMAKE_VERBOSE_MAKEFILE 1)

find_package(GTK2 COMPONENTS gtk gtkmm REQUIRED)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS GTK2::gtkmm
                       PROPERTIES IMPORTED_CONFIGURATIONS
                                  IMPORTED_LOCATION_RELEASE
                                  IMPORTED_LOCATION_DEBUG
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG
                                  INTERFACE_LINK_LIBRARIES
                                  INTERFACE_INCLUDE_DIRECTORIES
                                  INTERFACE_COMPILE_DEFINITIONS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(gtkmm-target WIN32 main.cpp helloworld.cpp helloworld.h)
target_link_libraries(gtkmm-target GTK2::gtkmm)

add_executable(gtkmm-all-libs WIN32 main.cpp helloworld.cpp helloworld.h)
target_link_libraries(gtkmm-all-libs ${GTK2_LIBRARIES})
target_include_directories(gtkmm-all-libs PRIVATE ${GTK2_INCLUDE_DIRS})
