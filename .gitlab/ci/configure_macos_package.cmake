set(CPACK_SYSTEM_NAME "macos-universal" CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_package_common.cmake")
