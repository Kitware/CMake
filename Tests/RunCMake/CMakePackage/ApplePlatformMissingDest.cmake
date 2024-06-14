include(CMakePackageConfigHelpers)
generate_apple_platform_selection_file(mylib-config-install.cmake
  #missing: INSTALL_DESTINATION lib/cmake/mylib
  )
