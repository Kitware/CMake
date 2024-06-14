include(apple-common.cmake)

include(CMakePackageConfigHelpers)
generate_apple_platform_selection_file(bad-platform-fatal-config-install.cmake
  INSTALL_DESTINATION lib/cmake/bad-platform-fatal
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bad-platform-fatal-config-install.cmake DESTINATION lib/cmake/bad-platform-fatal RENAME bad-platform-fatal-config.cmake)

generate_apple_architecture_selection_file(bad-arch-fatal-config-install.cmake
  INSTALL_DESTINATION lib/cmake/bad-arch-fatal
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/bad-arch-fatal-config-install.cmake DESTINATION lib/cmake/bad-arch-fatal RENAME bad-arch-fatal-config.cmake)
