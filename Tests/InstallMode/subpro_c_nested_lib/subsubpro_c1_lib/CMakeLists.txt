# This CMakeLists.txt is a nested subproject of the
# subproject C (ExternalProject_Add).

cmake_minimum_required(VERSION 3.20)
project(c1_lib_project VERSION 1.2.3 LANGUAGES CXX)

include(GNUInstallDirs)

add_library(the_c1_lib STATIC
  "include/c1_lib.h"
  "src/c1_lib.cpp"
)

target_include_directories(the_c1_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  DIRECTORY   "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
  TARGETS
    the_c1_lib
  EXPORT main
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

set(INSTALL_CMAKE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "cmake/PackageConfig.cmake.in"
  "${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${INSTALL_CMAKE_DIR}"
  PATH_VARS
    CMAKE_INSTALL_INCLUDEDIR
    CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file("${PROJECT_NAME}Version.cmake"
  VERSION       "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(
  EXPORT      main
  FILE        "${PROJECT_NAME}Targets.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Version.cmake"
  DESTINATION "${INSTALL_CMAKE_DIR}"
)
