find_package(PkgConfig REQUIRED)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/zot/lib/pkgconfig/zot.pc" "
prefix=${CMAKE_CURRENT_BINARY_DIR}/zot
libdir=\${prefix}/lib

Name: Zot
Description: Dummy packaget to test LIBRARY_DIR support
Version: 1.0
Libs: -L\${libdir} -lzot
")

# Create a "library" file to find in libdir.
set(CMAKE_FIND_LIBRARY_PREFIXES "prefix-")
set(CMAKE_FIND_LIBRARY_SUFFIXES "-suffix")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/zot/lib/prefix-zot-suffix")

# 'pkg-config --libs' drops -L flags in PKG_CONFIG_SYSTEM_LIBRARY_PATH by default.
set(ENV{PKG_CONFIG_SYSTEM_LIBRARY_PATH} "${CMAKE_CURRENT_BINARY_DIR}/zot/lib")

# 'pkgconf --libs' also drops -L flags in LIBRARY_PATH by default.
set(ENV{LIBRARY_PATH}                   "${CMAKE_CURRENT_BINARY_DIR}/zot/lib")

set(ENV{PKG_CONFIG_PATH}                "${CMAKE_CURRENT_BINARY_DIR}/zot/lib/pkgconfig")
pkg_check_modules(ZOT REQUIRED zot)

message(STATUS "ZOT_LIBRARIES='${ZOT_LIBRARIES}'")
message(STATUS "ZOT_LINK_LIBRARIES='${ZOT_LINK_LIBRARIES}'")
message(STATUS "ZOT_LDFLAGS='${ZOT_LDFLAGS}'")
