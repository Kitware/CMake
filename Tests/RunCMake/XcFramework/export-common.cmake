enable_language(C)

include(CMakePackageConfigHelpers)

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "tvOS" OR CMAKE_SYSTEM_NAME STREQUAL "watchOS" OR CMAKE_SYSTEM_NAME STREQUAL "visionOS")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "YES")
endif()

add_library(mylib STATIC mylib/mylib.c)
target_include_directories(mylib INTERFACE $<INSTALL_INTERFACE:include>)
set_property(TARGET mylib PROPERTY ARCHIVE_OUTPUT_DIRECTORY lib/${platform_name})

add_library(mylib-genex STATIC mylib/mylib.c)
target_include_directories(mylib-genex INTERFACE $<INSTALL_INTERFACE:include>)
set_property(TARGET mylib-genex PROPERTY ARCHIVE_OUTPUT_DIRECTORY lib/${platform_name})

install(TARGETS mylib mylib-genex DESTINATION lib/${platform_name} EXPORT mylib)
install(FILES mylib/include/mylib/mylib.h DESTINATION include/mylib)
export(SETUP mylib
  TARGET mylib XCFRAMEWORK_LOCATION lib/mylib.xcframework
  TARGET mylib-genex XCFRAMEWORK_LOCATION "$<BUILD_INTERFACE:lib/$<TARGET_PROPERTY:NAME>.xcframework>$<INSTALL_INTERFACE:lib2/$<TARGET_PROPERTY:NAME>.xcframework>"
  )
install(EXPORT mylib DESTINATION lib/${platform_name}/cmake/mylib FILE mylib-targets.cmake)
export(EXPORT mylib FILE lib/${platform_name}/cmake/mylib/mylib-targets.cmake)

configure_package_config_file(mylib-config.cmake.in mylib-config-sub.cmake INSTALL_DESTINATION lib/${platform_name}/cmake/mylib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mylib-config-sub.cmake DESTINATION lib/${platform_name}/cmake/mylib RENAME mylib-config.cmake)

configure_package_config_file(mylib-config.cmake.in lib/${platform_name}/cmake/mylib/mylib-config.cmake INSTALL_DESTINATION lib/${platform_name}/cmake/mylib)

generate_apple_platform_selection_file(mylib-config-top.cmake
  INSTALL_DESTINATION lib/cmake/mylib
  MACOS_INCLUDE_FILE lib/macos/cmake/mylib/mylib-config.cmake
  IOS_INCLUDE_FILE lib/ios/cmake/mylib/mylib-config.cmake
  IOS_SIMULATOR_INCLUDE_FILE lib/ios-simulator/cmake/mylib/mylib-config.cmake
  )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mylib-config-top.cmake DESTINATION lib/cmake/mylib RENAME mylib-config.cmake)

generate_apple_platform_selection_file(lib/cmake/mylib/mylib-config.cmake
  INSTALL_DESTINATION lib/cmake/mylib
  "${platform_arg}" lib/${platform_name}/cmake/mylib/mylib-config.cmake
  )
