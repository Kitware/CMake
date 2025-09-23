set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

cmake_pkg_config(IMPORT alpha REQUIRED
  NAME moe
)

if(TARGET PkgConfig::alpha)
  message("cmake_pkg_config target rename created PkgConfig::<package> target")
endif()

if(NOT TARGET PkgConfig::moe)
  message("cmake_pkg_config target rename failed to create PkgConfig::<name> target")
endif()
