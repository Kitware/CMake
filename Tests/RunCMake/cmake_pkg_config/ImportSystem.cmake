set(CMAKE_PKG_CONFIG_SYS_INCLUDE_DIRS /TestDirectories/Include)
set(CMAKE_PKG_CONFIG_SYS_LIB_DIRS /TestDirectories/Library)

cmake_pkg_config(IMPORT import-simple REQUIRED)

file(GENERATE
  OUTPUT import-system.txt
  CONTENT
"Include Directories: $<TARGET_PROPERTY:PkgConfig::import-simple,INTERFACE_INCLUDE_DIRECTORIES>
Link Directories: $<TARGET_PROPERTY:PkgConfig::import-simple,INTERFACE_LINK_DIRECTORIES>
"
)
