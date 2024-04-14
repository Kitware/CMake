set(CMAKE_INSTALL_DATADIR share)

include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pkg_a-config.cmake.in [[
@PACKAGE_INIT@
include("@PACKAGE_CMAKE_INSTALL_DATADIR@/pkg_a_included.cmake")
message(STATUS "Leaving pkg_a-config.cmake with PACKAGE_PREFIX_DIR = ${PACKAGE_PREFIX_DIR}")
]])
configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/pkg_a-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/install_pkg_a/pkg_a-config.cmake
  INSTALL_DESTINATION .
  PATH_VARS CMAKE_INSTALL_DATADIR
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install_pkg_a/share/pkg_a_included.cmake
  [[message(STATUS "Hello from pkg_a")]]
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pkg_b-config.cmake.in [[
@PACKAGE_INIT@
include(CMakeFindDependencyMacro)
message(STATUS "Before find_dependency: PACKAGE_PREFIX_DIR = ${PACKAGE_PREFIX_DIR}")
find_dependency(pkg_a NO_DEFAULT_PATH
    PATHS "@CMAKE_CURRENT_BINARY_DIR@/install_pkg_a"
)
message(STATUS "After find_dependency:  PACKAGE_PREFIX_DIR = ${PACKAGE_PREFIX_DIR}")
include("@PACKAGE_CMAKE_INSTALL_DATADIR@/pkg_b_included.cmake")
message(STATUS "Leaving pkg_b-config.cmake with PACKAGE_PREFIX_DIR = ${PACKAGE_PREFIX_DIR}")
]])
configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/pkg_b-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/install_pkg_b/pkg_b-config.cmake
  INSTALL_DESTINATION .
  PATH_VARS CMAKE_INSTALL_DATADIR
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install_pkg_b/share/pkg_b_included.cmake
  [[message(STATUS "Hello from pkg_b")]]
)

find_package(pkg_b REQUIRED NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_BINARY_DIR}/install_pkg_b
)
