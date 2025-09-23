set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

cmake_pkg_config(IMPORT echo REQUIRED)
cmake_pkg_config(IMPORT delta REQUIRED)
cmake_pkg_config(IMPORT charlie REQUIRED)
cmake_pkg_config(IMPORT bravo REQUIRED)
cmake_pkg_config(IMPORT alpha REQUIRED)

file(GENERATE
  OUTPUT import-requires.txt
  CONTENT
  "alpha: $<TARGET_PROPERTY:PkgConfig::alpha,INTERFACE_COMPILE_OPTIONS>
bravo: $<TARGET_PROPERTY:PkgConfig::bravo,INTERFACE_COMPILE_OPTIONS>
charlie: $<TARGET_PROPERTY:PkgConfig::charlie,INTERFACE_COMPILE_OPTIONS>
delta: $<TARGET_PROPERTY:PkgConfig::delta,INTERFACE_COMPILE_OPTIONS>
echo: $<TARGET_PROPERTY:PkgConfig::echo,INTERFACE_COMPILE_OPTIONS>
"
)
