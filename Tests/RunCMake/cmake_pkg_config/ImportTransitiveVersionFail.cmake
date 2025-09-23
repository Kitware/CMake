set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

cmake_pkg_config(POPULATE bravo)
cmake_pkg_config(IMPORT hotel)
