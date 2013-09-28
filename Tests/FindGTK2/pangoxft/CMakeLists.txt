cmake_minimum_required(VERSION 2.8)

project(pangoxft)

find_package(GTK2 COMPONENTS gtk REQUIRED)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS GTK2::pangoxft
                       PROPERTIES IMPORTED_CONFIGURATIONS
                                  IMPORTED_LOCATION_RELEASE
                                  IMPORTED_LOCATION_DEBUG
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG
                                  INTERFACE_LINK_LIBRARIES
                                  INTERFACE_INCLUDE_DIRECTORIES
                                  INTERFACE_COMPILE_DEFINITIONS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(pangoxft WIN32 main.c)
target_link_libraries(pangoxft GTK2::pangoxft)
