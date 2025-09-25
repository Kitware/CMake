cmake_pkg_config(
  EXTRACT foo
  STRICTNESS ${STRICTNESS}
  REQUIRED
)

cmake_pkg_config(
  EXTRACT empty-key
  STRICTNESS ${STRICTNESS}
  REQUIRED
)

cmake_pkg_config(
  EXTRACT no-name
  STRICTNESS ${STRICTNESS}
)

cmake_pkg_config(
  EXTRACT no-description
  STRICTNESS ${STRICTNESS}
)

cmake_pkg_config(
  EXTRACT no-version
  STRICTNESS ${STRICTNESS}
)

cmake_pkg_config(
  EXTRACT invalid
  STRICTNESS ${STRICTNESS}
)

cmake_pkg_config(
  EXTRACT cflags-lowercase-f
  STRICTNESS ${STRICTNESS}
)
message("Cflags: ${CMAKE_PKG_CONFIG_CFLAGS}")

set(CMAKE_PKG_CONFIG_CFLAGS)
cmake_pkg_config(
  EXTRACT cflags-uppercase-f
  STRICTNESS ${STRICTNESS}
)
message("CFlags: ${CMAKE_PKG_CONFIG_CFLAGS}")

set(CMAKE_PKG_CONFIG_CFLAGS)
cmake_pkg_config(
  EXTRACT cflags-bothcase-f
  STRICTNESS ${STRICTNESS}
)
message("Cflags: ${CMAKE_PKG_CONFIG_CFLAGS}")
