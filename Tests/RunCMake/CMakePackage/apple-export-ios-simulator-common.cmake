include(apple-export-common.cmake)

if(IOS_SIMULATOR_SELECT_ARCHS)
  set(IOS_SIMULATOR_SELECT_FILES "${IOS_SIMULATOR_SELECT_ARCHS}")
  list(TRANSFORM IOS_SIMULATOR_SELECT_FILES PREPEND "lib/ios-simulator-")
  list(TRANSFORM IOS_SIMULATOR_SELECT_FILES APPEND "/cmake/mylib/mylib-targets.cmake")
  generate_apple_architecture_selection_file(mylib-select-arch-install.cmake
    INSTALL_DESTINATION lib/ios-simulator/cmake/mylib
    INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
    SINGLE_ARCHITECTURES ${IOS_SIMULATOR_SELECT_ARCHS}
    SINGLE_ARCHITECTURE_INCLUDE_FILES ${IOS_SIMULATOR_SELECT_FILES}
    UNIVERSAL_ARCHITECTURES ${IOS_SIMULATOR_SELECT_ARCHS} $(ARCHS_STANDARD)
    UNIVERSAL_INCLUDE_FILE "lib/ios-simulator/cmake/mylib/mylib-targets.cmake"
    )
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mylib-select-arch-install.cmake DESTINATION lib/ios-simulator/cmake/mylib RENAME mylib-select-arch.cmake)
endif()
