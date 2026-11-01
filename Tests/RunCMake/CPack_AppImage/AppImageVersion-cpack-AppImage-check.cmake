# CPACK_APPIMAGE_VERSION given explicitly overrides CPACK_PACKAGE_VERSION.
set(expected_version "4.5.6-test")
include("${CMAKE_CURRENT_LIST_DIR}/AppImageVersionCheck.cmake")
