set(CMAKE_PKG_CONFIG_SYSROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

cmake_pkg_config(IMPORT import-simple REQUIRED)

file(GENERATE
  OUTPUT import-simple.txt
  CONTENT
"Import Simple Found: ${PKGCONFIG_import-simple_FOUND}
Include Directories: $<TARGET_PROPERTY:PkgConfig::import-simple,INTERFACE_INCLUDE_DIRECTORIES>
Compile Options: $<TARGET_PROPERTY:PkgConfig::import-simple,INTERFACE_COMPILE_OPTIONS>
Link Directories: $<TARGET_PROPERTY:PkgConfig::import-simple,INTERFACE_LINK_DIRECTORIES>
Link Libraries: $<TARGET_PROPERTY:PkgConfig::import-simple,INTERFACE_LINK_LIBRARIES>
Link Options: $<TARGET_PROPERTY:PkgConfig::import-simple,INTERFACE_LINK_OPTIONS>
"
)
