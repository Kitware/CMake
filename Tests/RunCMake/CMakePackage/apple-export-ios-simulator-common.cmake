include(apple-export-common.cmake)

if(IOS_SIMULATOR_SELECT_ARCHS)
  generate_apple_architecture_selection_file(mylib-select-arch-install.cmake
    INSTALL_DESTINATION lib/ios-simulator/cmake/mylib
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
    SINGLE_ARCHITECTURES "${IOS_SIMULATOR_SELECT_ARCHS}"
    SINGLE_ARCHITECTURE_INCLUDE_FILES "lib/ios-simulator-arm64/cmake/mylib/mylib-targets.cmake;lib/ios-simulator-x86_64/cmake/mylib/mylib-targets.cmake"
    UNIVERSAL_ARCHITECTURES "${IOS_SIMULATOR_SELECT_ARCHS}"
    UNIVERSAL_INCLUDE_FILE "lib/ios-simulator/cmake/mylib/mylib-targets.cmake"
    )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mylib-select-arch-install.cmake DESTINATION lib/ios-simulator/cmake/mylib RENAME mylib-select-arch.cmake)
endif()
