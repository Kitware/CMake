set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

cmake_pkg_config(IMPORT golf)
message("Import Golf Found: ${PKGCONFIG_golf_FOUND}")
cmake_pkg_config(IMPORT golf REQUIRED)
