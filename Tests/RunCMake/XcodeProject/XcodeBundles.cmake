# check if Xcode and CMake have the same understanding of Bundle layout

cmake_minimum_required(VERSION 3.3)
enable_language(C)

if(TEST_IOS)
  set(CMAKE_OSX_SYSROOT iphoneos)
  set(CMAKE_OSX_ARCHITECTURES "armv7")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")
endif(TEST_IOS)

if(TEST_WATCHOS)
  set(CMAKE_OSX_SYSROOT watchos)
  set(CMAKE_OSX_ARCHITECTURES "armv7k")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "YES")
endif()

if(TEST_TVOS)
  set(CMAKE_OSX_SYSROOT appletvos)
  set(CMAKE_OSX_ARCHITECTURES "arm64")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
  set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "YES")
endif()

# App Bundle

add_executable(AppBundle MACOSX_BUNDLE main.m)

add_custom_target(AppBundleTest ALL
  COMMAND ${CMAKE_COMMAND} -E copy
    "$<TARGET_FILE:AppBundle>" "$<TARGET_FILE:AppBundle>.old")

add_dependencies(AppBundleTest AppBundle)

# with custom extension

if (NOT TEST_IOS AND NOT TEST_WATCHOS AND NOT TEST_TVOS)
  add_executable(AppBundleExt MACOSX_BUNDLE main.m)
  set_target_properties(AppBundleExt PROPERTIES BUNDLE_EXTENSION "foo")
  install(TARGETS AppBundleExt BUNDLE DESTINATION FooExtension)

  add_custom_target(AppBundleExtTest ALL
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:AppBundleExt>" "$<TARGET_FILE:AppBundleExt>.old")

  add_dependencies(AppBundleExtTest AppBundleExt)
endif()

# Framework (not supported for iOS on Xcode < 6)

if(NOT TEST_IOS OR NOT XCODE_VERSION VERSION_LESS 6)
  add_library(Framework SHARED main.c)
  set_target_properties(Framework PROPERTIES FRAMEWORK TRUE)

  add_custom_target(FrameworkTest ALL
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:Framework>" "$<TARGET_FILE:Framework>.old")

  add_dependencies(FrameworkTest Framework)

  # with custom extension

  add_library(FrameworkExt SHARED main.c)
  set_target_properties(FrameworkExt PROPERTIES FRAMEWORK TRUE)
  set_target_properties(FrameworkExt PROPERTIES BUNDLE_EXTENSION "foo")
  install(TARGETS FrameworkExt FRAMEWORK DESTINATION FooExtension)

  add_custom_target(FrameworkExtTest ALL
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:FrameworkExt>" "$<TARGET_FILE:FrameworkExt>.old")

  add_dependencies(FrameworkExtTest FrameworkExt)
endif()

# Bundle

if(NOT CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE)
  add_library(Bundle MODULE main.c)
  set_target_properties(Bundle PROPERTIES BUNDLE TRUE)

  add_custom_target(BundleTest ALL
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:Bundle>" "$<TARGET_FILE:Bundle>.old")

  add_dependencies(BundleTest Bundle)

  # with custom extension

  add_library(BundleExt MODULE main.c)
  set_target_properties(BundleExt PROPERTIES BUNDLE TRUE)
  set_target_properties(BundleExt PROPERTIES BUNDLE_EXTENSION "foo")
  install(TARGETS BundleExt LIBRARY DESTINATION FooExtension)

  add_custom_target(BundleExtTest ALL
    COMMAND ${CMAKE_COMMAND} -E copy
      "$<TARGET_FILE:BundleExt>" "$<TARGET_FILE:BundleExt>.old")

  add_dependencies(BundleExtTest BundleExt)
endif()
