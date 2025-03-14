set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

cmake_pkg_config(IMPORT alpha REQUIRED
  NAME larry
)

set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/AltRequiresPackages)

cmake_pkg_config(IMPORT alpha REQUIRED
  NAME curly
)

cmake_pkg_config(IMPORT alpha REQUIRED
  NAME moe
  PREFIX moe
)

set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

cmake_pkg_config(IMPORT alpha REQUIRED
  NAME shemp
  PREFIX moe
)

file(GENERATE
  OUTPUT import-prefix.txt
  CONTENT
"larry: $<TARGET_PROPERTY:PkgConfig::larry,INTERFACE_COMPILE_OPTIONS>
curly: $<TARGET_PROPERTY:PkgConfig::curly,INTERFACE_COMPILE_OPTIONS>
moe: $<TARGET_PROPERTY:PkgConfig::moe,INTERFACE_COMPILE_OPTIONS>
shemp: $<TARGET_PROPERTY:PkgConfig::shemp,INTERFACE_COMPILE_OPTIONS>
"
)
