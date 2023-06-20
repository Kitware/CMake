set(CMake_DOC_ARTIFACT_PREFIX "$ENV{CI_PROJECT_DIR}/build/install-doc" CACHE PATH "")

# Set up install destinations as expected by the packaging scripts.
set(CMAKE_INSTALL_PREFIX "/" CACHE PATH "")
set(CMAKE_DOC_DIR "doc/cmake" CACHE STRING "")

# Settings for CMake packages for macOS.
set(CPACK_DMG_FORMAT "UDBZ" CACHE STRING "")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")
set(BUILD_CursesDialog "ON" CACHE BOOL "")
set(BUILD_QtDialog "TRUE" CACHE BOOL "")
set(CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL "3" CACHE STRING "")
set(CMake_INSTALL_DEPENDENCIES "ON" CACHE BOOL "")
set(CMAKE_SKIP_RPATH "TRUE" CACHE BOOL "")
set(CMake_TEST_BOOTSTRAP OFF CACHE BOOL "")
set(CMake_TEST_NO_FindPackageModeMakefileTest "TRUE" CACHE BOOL "")

# XXX(sccache): restore sccache when it works for multiple architectures:
# https://github.com/mozilla/sccache/issues/847
set(configure_no_sccache 1)

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/configure_common.cmake")
