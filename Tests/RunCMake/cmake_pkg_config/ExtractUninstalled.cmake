cmake_pkg_config(EXTRACT qux)

message(${CMAKE_PKG_CONFIG_CFLAGS})

cmake_pkg_config(
  EXTRACT qux
  DISABLE_UNINSTALLED ON
)

message(${CMAKE_PKG_CONFIG_CFLAGS})
