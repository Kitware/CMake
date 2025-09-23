cmake_pkg_config(EXTRACT all-extract-fields)

message("Name: ${CMAKE_PKG_CONFIG_NAME}")
message("Description: ${CMAKE_PKG_CONFIG_DESCRIPTION}")
message("Version: ${CMAKE_PKG_CONFIG_VERSION}")

message("Conflicts: ${CMAKE_PKG_CONFIG_CONFLICTS}")
message("Provides: ${CMAKE_PKG_CONFIG_PROVIDES}")

message("Requires: ${CMAKE_PKG_CONFIG_REQUIRES}")
message("Requires.private: ${CMAKE_PKG_CONFIG_REQUIRES_PRIVATE}")

message("Cflags: ${CMAKE_PKG_CONFIG_CFLAGS}")
message("Includes: ${CMAKE_PKG_CONFIG_INCLUDES}")
message("CompileOptions: ${CMAKE_PKG_CONFIG_COMPILE_OPTIONS}")

message("Cflags.private: ${CMAKE_PKG_CONFIG_CFLAGS_PRIVATE}")
message("Includes.private: ${CMAKE_PKG_CONFIG_INCLUDES_PRIVATE}")
message("CompileOptions.private: ${CMAKE_PKG_CONFIG_COMPILE_OPTIONS_PRIVATE}")

message("Libs: ${CMAKE_PKG_CONFIG_LIBS}")
message("LibDirs: ${CMAKE_PKG_CONFIG_LIBDIRS}")
message("LibNames: ${CMAKE_PKG_CONFIG_LIBNAMES}")
message("LinkOptions: ${CMAKE_PKG_CONFIG_LINK_OPTIONS}")

message("Libs.private: ${CMAKE_PKG_CONFIG_LIBS_PRIVATE}")
message("LibDirs.private: ${CMAKE_PKG_CONFIG_LIBDIRS_PRIVATE}")
message("LibNames.private: ${CMAKE_PKG_CONFIG_LIBNAMES_PRIVATE}")
message("LinkOptions.private: ${CMAKE_PKG_CONFIG_LINK_OPTIONS_PRIVATE}")
