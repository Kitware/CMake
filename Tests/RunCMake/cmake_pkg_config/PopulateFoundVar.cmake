set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

cmake_pkg_config(POPULATE alpha)
message("Found Alpha: ${PKGCONFIG_alpha_FOUND}")

cmake_pkg_config(POPULATE foxtrot)
message("Found Foxtrot: ${PKGCONFIG_foxtrot_FOUND}")
