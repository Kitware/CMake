if(NOT "$ENV{CMAKE_CI_PACKAGE}" MATCHES "^(dev)?$")
  configure_file(${CMAKE_CURRENT_LIST_DIR}/package_info.cmake.in ${CMake_BINARY_DIR}/ci_package_info.cmake @ONLY)
endif()
