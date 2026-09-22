set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}/pc")
cmake_pkg_config(IMPORT cps-pkg-test REQUIRED)

if(imported)
  add_library(cps-pkg-test::bar INTERFACE IMPORTED)
  set_property(TARGET cps-pkg-test::bar PROPERTY
    EXPORT_FIND_PACKAGE_NAME cps-pkg-test)
else()
  add_library(bar INTERFACE)
  add_library(cps-pkg-test::bar ALIAS bar)
  install(TARGETS bar EXPORT dependency)
  export(PACKAGE_INFO cps-pkg-test EXPORT dependency)
endif()

add_library(consumer INTERFACE)
if(pkg_config_first)
  target_link_libraries(consumer INTERFACE
    PkgConfig::cps-pkg-test cps-pkg-test::bar)
else()
  target_link_libraries(consumer INTERFACE
    cps-pkg-test::bar PkgConfig::cps-pkg-test)
endif()

install(TARGETS consumer EXPORT consumer)
export(PACKAGE_INFO consumer EXPORT consumer)
