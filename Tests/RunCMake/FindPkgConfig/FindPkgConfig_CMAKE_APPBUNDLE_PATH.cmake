# Needed for CMAKE_SYSTEM_NAME, CMAKE_LIBRARY_ARCHITECTURE, FIND_LIBRARY_USE_LIB32_PATHS and FIND_LIBRARY_USE_LIB64_PATHS
enable_language(C)

# Prepare environment and variables
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)
set(CMAKE_APPBUNDLE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/pc-foo")
set(PKG_CONFIG_EXECUTABLE "${CMAKE_CURRENT_SOURCE_DIR}/dummy-pkg-config.sh")
set(ENV{CMAKE_APPBUNDLE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/pc-bar:/this/directory/should/not/exist/in/the/filesystem")
set(ENV{PKG_CONFIG_PATH} "/baz")

find_package(PkgConfig)

set(expected_path "/baz:${CMAKE_CURRENT_SOURCE_DIR}/pc-foo/lib/pkgconfig:${CMAKE_CURRENT_SOURCE_DIR}/pc-bar/lib/pkgconfig")

pkg_check_modules(FOO "${expected_path}")

if(NOT FOO_FOUND)
  message(FATAL_ERROR "Expected PKG_CONFIG_PATH: \"${expected_path}\".")
endif()
