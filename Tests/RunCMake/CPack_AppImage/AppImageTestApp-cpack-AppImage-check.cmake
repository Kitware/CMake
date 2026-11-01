# CPACK_APPIMAGE_VERSION defaults to CPACK_PACKAGE_VERSION.
set(expected_version "1.2.3")
include("${CMAKE_CURRENT_LIST_DIR}/AppImageVersionCheck.cmake")
