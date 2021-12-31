find_package(PkgConfig REQUIRED)

set(ROOT "${CMAKE_CURRENT_BINARY_DIR}/root")
string(REPLACE " " "\\ " ESCAPED_ROOT "${ROOT}")
set(LIB_DIR "${ROOT}/lib")
set(PKGCONFIG_DIR "${LIB_DIR}/pkgconfig")

file(WRITE "${PKGCONFIG_DIR}/imm.pc" "
prefix=${ESCAPED_ROOT}
libdir=\${prefix}/lib

Name: Immediate
Description: Dummy package to test *LINK_LIBRARIES support
Version: 1.0
Libs: -L\${libdir} -limm
Libs.private: -ltrns
")
file(WRITE "${PKGCONFIG_DIR}/trns.pc" "
prefix=${ESCAPED_ROOT}
libdir=\${prefix}/lib

Name: Transitive
Description: Dummy package to test *LINK_LIBRARIES support
Version: 1.0
Libs: -L\${libdir} -ltrns
")

set(shared_lib_prefix "dyprefix-")
set(shared_lib_suffix "-dysuffix")
set(static_lib_prefix "stprefix-")
set(static_lib_suffix "-stsuffix")

set(CMAKE_SHARED_LIBRARY_PREFIX ${shared_lib_prefix})
set(CMAKE_SHARED_LIBRARY_SUFFIX ${shared_lib_suffix})
set(CMAKE_STATIC_LIBRARY_PREFIX ${static_lib_prefix})
set(CMAKE_STATIC_LIBRARY_SUFFIX ${static_lib_suffix})

# Create "library" files to find in libdir.
foreach(lib imm trns)
  foreach(variant shared static)
    file(WRITE "${LIB_DIR}/${${variant}_lib_prefix}${lib}${${variant}_lib_suffix}")
  endforeach()
endforeach()

set(ENV{PKG_CONFIG_PATH}                "${PKGCONFIG_DIR}")
pkg_check_modules(IMM REQUIRED imm)

message(STATUS "IMM_LIBRARIES='${IMM_LIBRARIES}'")
message(STATUS "IMM_LINK_LIBRARIES='${IMM_LINK_LIBRARIES}'")
message(STATUS "IMM_LDFLAGS='${IMM_LDFLAGS}'")
message(STATUS "IMM_STATIC_LIBRARIES='${IMM_STATIC_LIBRARIES}'")
message(STATUS "IMM_STATIC_LINK_LIBRARIES='${IMM_STATIC_LINK_LIBRARIES}'")
message(STATUS "IMM_STATIC_LDFLAGS='${IMM_STATIC_LDFLAGS}'")
