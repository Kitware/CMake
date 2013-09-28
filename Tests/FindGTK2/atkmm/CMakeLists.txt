cmake_minimum_required(VERSION 2.8)

project(atkmm)

find_package(GTK2 COMPONENTS gtk gtkmm REQUIRED)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS GTK2::atkmm
                       PROPERTIES IMPORTED_CONFIGURATIONS
                                  IMPORTED_LOCATION_RELEASE
                                  IMPORTED_LOCATION_DEBUG
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE
                                  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG
                                  INTERFACE_LINK_LIBRARIES
                                  INTERFACE_INCLUDE_DIRECTORIES
                                  INTERFACE_COMPILE_DEFINITIONS)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(atkmm WIN32 main.cpp)
target_link_libraries(atkmm GTK2::atkmm)
