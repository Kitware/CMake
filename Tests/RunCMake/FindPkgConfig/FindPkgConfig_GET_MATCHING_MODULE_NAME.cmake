# Prepare environment to reuse bletch.pc
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pc-bletch/lib/pkgconfig" PC_PATH)
if(UNIX)
  string(REPLACE "\\ " " " PC_PATH "${PC_PATH}")
endif()
set(ENV{PKG_CONFIG_PATH} "${PC_PATH}")

find_package(PkgConfig REQUIRED)
pkg_search_module(FOO REQUIRED foo bletch bar)

if(NOT FOO_MODULE_NAME STREQUAL "bletch")
  message(FATAL_ERROR "Wrong value for FOO_MODULE_NAME. Expected: bletch, got: ${FOO_MODULE_NAME}")
endif()

pkg_get_variable(FOO_JACKPOT ${FOO_MODULE_NAME} jackpot)

if(NOT FOO_JACKPOT STREQUAL "bletch-lives")
  message(FATAL_ERROR "Wrong value for FOO_JACKPOT. Expected: bletch-lives, got: ${FOO_JACKPOT}")
endif()

unset(FOO_MODULE_NAME)

# verify variable gets also set on subsequent run
pkg_search_module(FOO REQUIRED foo bletch bar)

if(NOT FOO_MODULE_NAME STREQUAL "bletch")
  message(FATAL_ERROR "Wrong value for FOO_MODULE_NAME on second run. Expected: bletch, got: ${FOO_MODULE_NAME}")
endif()
