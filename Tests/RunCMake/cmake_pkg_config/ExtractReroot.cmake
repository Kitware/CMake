cmake_pkg_config(
  EXTRACT relocate
  PC_SYSROOT_DIR /NewRoot
)

message("Cflags: ${CMAKE_PKG_CONFIG_CFLAGS}")
message("Includes: ${CMAKE_PKG_CONFIG_INCLUDES}")

message("Libs: ${CMAKE_PKG_CONFIG_LIBS}")
message("LibDirs: ${CMAKE_PKG_CONFIG_LIBDIRS}")
