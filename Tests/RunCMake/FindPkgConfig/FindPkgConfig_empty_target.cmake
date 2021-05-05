# Prepare environment to reuse empty.pc
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pc-empty/lib/pkgconfig" PC_PATH)
if(UNIX)
  string(REPLACE "\\ " " " PC_PATH "${PC_PATH}")
endif()
set(ENV{PKG_CONFIG_PATH} "${PC_PATH}")

find_package(PkgConfig REQUIRED)
pkg_search_module(Empty REQUIRED empty IMPORTED_TARGET)

if(NOT Empty_MODULE_NAME STREQUAL "empty")
  message(FATAL_ERROR "Wrong value for Empty_MODULE_NAME. Expected: empty, got: ${Empty_MODULE_NAME}")
endif()

if(NOT TARGET PkgConfig::Empty)
  message(FATAL_ERROR "PkgConfig::Empty target not created")
endif()
