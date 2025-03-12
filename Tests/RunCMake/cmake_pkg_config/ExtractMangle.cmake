set(CMAKE_PKG_CONFIG_SYS_INCLUDE_DIRS /Alpha)
set(CMAKE_PKG_CONFIG_SYS_LIB_DIRS /Delta)

cmake_pkg_config(EXTRACT relocate)

message("Cflags: ${CMAKE_PKG_CONFIG_CFLAGS}")
message("Includes: ${CMAKE_PKG_CONFIG_INCLUDES}")

message("Libs: ${CMAKE_PKG_CONFIG_LIBS}")
message("LibDirs: ${CMAKE_PKG_CONFIG_LIBDIRS}")

cmake_pkg_config(
  EXTRACT relocate
  ALLOW_SYSTEM_INCLUDES ON
  ALLOW_SYSTEM_LIBS ON
)

message("Cflags: ${CMAKE_PKG_CONFIG_CFLAGS}")
message("Includes: ${CMAKE_PKG_CONFIG_INCLUDES}")

message("Libs: ${CMAKE_PKG_CONFIG_LIBS}")
message("LibDirs: ${CMAKE_PKG_CONFIG_LIBDIRS}")
