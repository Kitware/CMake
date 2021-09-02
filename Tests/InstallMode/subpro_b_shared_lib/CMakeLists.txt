# This CMakeLists.txt is part of the subproject B (ExternalProject_Add).

cmake_minimum_required(VERSION 3.20)
project(shared_lib_project VERSION 2.3.4 LANGUAGES CXX)

include(GNUInstallDirs)
include(GenerateExportHeader)

add_library(the_shared_lib SHARED
  "include/shared_lib.h"
  "src/shared_lib.cpp"
)

generate_export_header(the_shared_lib
  BASE_NAME shared_lib
  EXPORT_FILE_NAME include/shared_lib_export.h
)

set_target_properties(the_shared_lib
  PROPERTIES
    VERSION   "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION}"
)

target_include_directories(the_shared_lib PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/"
    "${CMAKE_CURRENT_BINARY_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
  TARGETS
    the_shared_lib
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
