# check if Xcode and CMake have the same understanding of Bundle layout

cmake_minimum_required(VERSION 3.3)
enable_language(C)

if(TEST_IOS)
  set(CMAKE_OSX_SYSROOT iphoneos)
  set(CMAKE_OSX_ARCHITECTURES "armv7")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
endif(TEST_IOS)

# App Bundle

add_executable(AppBundle MACOSX_BUNDLE main.m)

add_custom_target(AppBundleTest ALL
  COMMAND ${CMAKE_COMMAND} -E copy
    "$<TARGET_FILE:AppBundle>" "$<TARGET_FILE:AppBundle>.old")

add_dependencies(AppBundleTest AppBundle)

# Framework (not supported for iOS on Xcode < 6)

if(NOT TEST_IOS OR NOT XCODE_VERSION VERSION_LESS 6)
  add_library(Framework SHARED main.c)
  set_target_properties(Framework PROPERTIES FRAMEWORK TRUE)

  add_custom_target(FrameworkTest ALL
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:Framework>" "$<TARGET_FILE:Framework>.old")

  add_dependencies(FrameworkTest Framework)
endif()

# Bundle

add_library(Bundle MODULE main.c)
set_target_properties(Bundle PROPERTIES BUNDLE TRUE)

add_custom_target(BundleTest ALL
  COMMAND ${CMAKE_COMMAND} -E copy
    "$<TARGET_FILE:Bundle>" "$<TARGET_FILE:Bundle>.old")

add_dependencies(BundleTest Bundle)
