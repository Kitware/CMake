include(apple-common.cmake)

include(CMakePackageConfigHelpers)
generate_apple_platform_selection_file(bad-platform-capture-config-install.cmake
  INSTALL_DESTINATION lib/cmake/bad-platform-capture
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
  ERROR_VARIABLE bad-platform-capture_unsupported
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bad-platform-capture-config-install.cmake DESTINATION lib/cmake/bad-platform-capture RENAME bad-platform-capture-config.cmake)

generate_apple_architecture_selection_file(bad-arch-capture-config-install.cmake
  INSTALL_DESTINATION lib/cmake/bad-arch-capture
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
  ERROR_VARIABLE bad-arch-capture_unsupported
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bad-arch-capture-config-install.cmake DESTINATION lib/cmake/bad-arch-capture RENAME bad-arch-capture-config.cmake)
