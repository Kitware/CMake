set("${CMAKE_FIND_PACKAGE_NAME}_MODULE" "${CMAKE_CURRENT_LIST_FILE}")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NAMEMISMATCH "old signature" "${CMAKE_FIND_PACKAGE_NAME}_MODULE")
set("${CMAKE_FIND_PACKAGE_NAME}_FOUND" 1)
