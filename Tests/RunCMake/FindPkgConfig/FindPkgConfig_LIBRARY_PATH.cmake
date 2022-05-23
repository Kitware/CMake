find_package(PkgConfig REQUIRED)

set(ROOT "${CMAKE_CURRENT_BINARY_DIR}/root")
string(REPLACE " " "\\ " ESCAPED_ROOT "${ROOT}")
set(LIB_DIR "${ROOT}/lib")
set(PKGCONFIG_DIR "${LIB_DIR}/pkgconfig")

file(WRITE "${PKGCONFIG_DIR}/zot.pc" "
prefix=${ESCAPED_ROOT}
libdir=\${prefix}/lib

Name: Zot
Description: Dummy package to test LIBRARY_DIR support
Version: 1.0
Libs: -L\${libdir} -lzot
")

set(shared_lib_prefix "dyprefix-")
set(shared_lib_suffix "-dysuffix")
set(static_lib_prefix "stprefix-")
set(static_lib_suffix "-stsuffix")

set(CMAKE_SHARED_LIBRARY_PREFIX ${shared_lib_prefix})
set(CMAKE_SHARED_LIBRARY_SUFFIX ${shared_lib_suffix})
set(CMAKE_STATIC_LIBRARY_PREFIX ${static_lib_prefix})
set(CMAKE_STATIC_LIBRARY_SUFFIX ${static_lib_suffix})

# Create a "library" file to find in libdir.
foreach(variant shared static)
  file(WRITE "${LIB_DIR}/${${variant}_lib_prefix}zot${${variant}_lib_suffix}")
endforeach()

# 'pkg-config --libs' drops -L flags in PKG_CONFIG_SYSTEM_LIBRARY_PATH by default.
set(ENV{PKG_CONFIG_SYSTEM_LIBRARY_PATH} "${LIB_DIR}")

# 'pkgconf --libs' also drops -L flags in LIBRARY_PATH by default.
set(ENV{LIBRARY_PATH}                   "${LIB_DIR}")

set(ENV{PKG_CONFIG_PATH}                "${PKGCONFIG_DIR}")
pkg_check_modules(ZOT REQUIRED zot)

message(STATUS "ZOT_LIBRARIES='${ZOT_LIBRARIES}'")
message(STATUS "ZOT_LINK_LIBRARIES='${ZOT_LINK_LIBRARIES}'")
message(STATUS "ZOT_LDFLAGS='${ZOT_LDFLAGS}'")
message(STATUS "ZOT_STATIC_LIBRARIES='${ZOT_STATIC_LIBRARIES}'")
message(STATUS "ZOT_STATIC_LINK_LIBRARIES='${ZOT_STATIC_LINK_LIBRARIES}'")
message(STATUS "ZOT_STATIC_LDFLAGS='${ZOT_STATIC_LDFLAGS}'")
