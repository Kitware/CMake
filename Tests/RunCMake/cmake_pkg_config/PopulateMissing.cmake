set(CMAKE_PKG_CONFIG_PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot/RequiresPackages)

add_library(native-foxtrot INTERFACE)
target_compile_options(native-foxtrot INTERFACE Foxtrot)

cmake_pkg_config(
  POPULATE golf
  BIND_PC_REQUIRES
    foxtrot=native-foxtrot
)

cmake_pkg_config(IMPORT juliet)

file(GENERATE
  OUTPUT populate-missing.txt
  CONTENT
  "juliet: $<TARGET_PROPERTY:PkgConfig::juliet,INTERFACE_COMPILE_OPTIONS>
"
)
