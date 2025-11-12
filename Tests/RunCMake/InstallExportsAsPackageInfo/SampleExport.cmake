cmake_minimum_required(VERSION 4.2)

project(farm VERSION 1.2.3 COMPAT_VERSION 1.1.0)

set(CMAKE_INSTALL_EXPORTS_AS_PACKAGE_INFO
  cow:farm//cps
  pig:farm/aextra/cps
)
set(cow_EXPORT_PACKAGE_INFO_VERSION @farm_VERSION@)
set(cow_EXPORT_PACKAGE_INFO_COMPAT_VERSION @farm_COMPAT_VERSION@)
set(cow_EXPORT_PACKAGE_INFO_VERSION_SCHEMA "simple")
set(cow_EXPORT_PACKAGE_INFO_LICENSE "Apache-2.0")
set(cow_EXPORT_PACKAGE_INFO_DEFAULT_LICENSE "BSD-3-Clause")
set(cow_EXPORT_PACKAGE_INFO_DEFAULT_CONFIGURATIONS "Small;Large")

add_library(cow INTERFACE)
add_library(pig INTERFACE)

install(TARGETS cow EXPORT cow)
install(TARGETS pig EXPORT pig)
install(EXPORT cow FILE farm-targets.cmake DESTINATION .)
install(EXPORT pig FILE farm-targets-extra.cmake DESTINATION .)
