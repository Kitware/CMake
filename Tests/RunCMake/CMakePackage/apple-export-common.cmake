include(apple-common.cmake)

add_library(mylib STATIC src/mylib.c)
target_sources(mylib PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include FILES include/mylib.h)
install(TARGETS mylib EXPORT mylib-targets FILE_SET HEADERS ARCHIVE DESTINATION lib/${platform_name})

install(EXPORT mylib-targets DESTINATION lib/${platform_name}/cmake/mylib)

if(IOS_SIMULATOR_SELECT_ARCHS)
  set(IOS_SIMULATOR_INCLUDE_FILE lib/ios-simulator/cmake/mylib/mylib-select-arch.cmake)
else()
  set(IOS_SIMULATOR_INCLUDE_FILE lib/ios-simulator/cmake/mylib/mylib-targets.cmake)
endif()

include(CMakePackageConfigHelpers)
generate_apple_platform_selection_file(mylib-config-install.cmake
  INSTALL_DESTINATION lib/cmake/mylib
  INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}
  MACOS_INCLUDE_FILE lib/macos/cmake/mylib/mylib-targets.cmake
  IOS_INCLUDE_FILE lib/ios/cmake/mylib/mylib-targets.cmake
  IOS_SIMULATOR_INCLUDE_FILE ${IOS_SIMULATOR_INCLUDE_FILE}
  TVOS_INCLUDE_FILE lib/tvos/cmake/mylib/mylib-targets.cmake
  TVOS_SIMULATOR_INCLUDE_FILE lib/tvos-simulator/cmake/mylib/mylib-targets.cmake
  VISIONOS_INCLUDE_FILE lib/watchos/cmake/mylib/mylib-targets.cmake
  VISIONOS_SIMULATOR_INCLUDE_FILE lib/watchos-simulator/cmake/mylib/mylib-targets.cmake
  WATCHOS_INCLUDE_FILE lib/watchos/cmake/mylib/mylib-targets.cmake
  WATCHOS_SIMULATOR_INCLUDE_FILE lib/watchos-simulator/cmake/mylib/mylib-targets.cmake
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mylib-config-install.cmake DESTINATION lib/cmake/mylib RENAME mylib-config.cmake)
