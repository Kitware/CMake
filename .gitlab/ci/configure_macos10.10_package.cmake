set(CPACK_SYSTEM_NAME "macos10.10-universal" CACHE STRING "")
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "")

include("${CMAKE_CURRENT_LIST_DIR}/configure_macos_package_common.cmake")
