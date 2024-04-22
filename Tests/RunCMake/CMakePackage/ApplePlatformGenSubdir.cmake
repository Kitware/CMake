set(CMAKE_INSTALL_DATADIR share)
set(SWITCH_DIR platform/cmake)

include(CMakePackageConfigHelpers)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/pkg_a-config.cmake.in [[
@PACKAGE_INIT@
include("@PACKAGE_SWITCH_DIR@/platform-switch.cmake")
include("@PACKAGE_CMAKE_INSTALL_DATADIR@/pkg_a_included.cmake")
]])
configure_package_config_file(
  ${CMAKE_CURRENT_BINARY_DIR}/pkg_a-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/install/pkg_a-config.cmake
  INSTALL_DESTINATION .
  PATH_VARS CMAKE_INSTALL_DATADIR SWITCH_DIR
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/${CMAKE_INSTALL_DATADIR}/pkg_a_included.cmake
  [[message(STATUS "Hello from pkg_a")]]
)

# To expose re-using the same package prefix variable, we need to use a
# different install prefix. This is really contrived and not representative of
# what a package should do.
generate_apple_platform_selection_file(
  ${CMAKE_CURRENT_BINARY_DIR}/install/platform/cmake/platform-switch.cmake
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/platform
  INSTALL_DESTINATION cmake
  MACOS_INCLUDE_FILE cmake/switch_included.cmake  # relative to install prefix
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/platform/cmake/switch_included.cmake
[[
message(STATUS "Hello from platform switch")
include("${CMAKE_CURRENT_LIST_DIR}/../arch/cmake/arch-switch.cmake")
]]
)

generate_apple_architecture_selection_file(
  ${CMAKE_CURRENT_BINARY_DIR}/install/platform/arch/cmake/arch-switch.cmake
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/platform/arch
  INSTALL_DESTINATION cmake
  UNIVERSAL_ARCHITECTURES i386 x86_64 arm64 $(ARCHS_STANDARD)
  UNIVERSAL_INCLUDE_FILE cmake/switch_included.cmake  # relative to install prefix
)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install/platform/arch/cmake/switch_included.cmake
  [[message(STATUS "Hello from arch switch")]]
)

find_package(pkg_a REQUIRED NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_BINARY_DIR}/install
)
