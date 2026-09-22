set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}/pc")

cmake_pkg_config(IMPORT cps-pkg-test REQUIRED)

add_library(foo INTERFACE)
target_link_libraries(foo INTERFACE PkgConfig::cps-pkg-test)

# Multiple components may require the same package in the same domain.
add_library(bar INTERFACE)
target_link_libraries(bar INTERFACE PkgConfig::cps-pkg-test)

install(TARGETS foo bar EXPORT foo DESTINATION .)
install(PACKAGE_INFO foo EXPORT foo DESTINATION cps)
