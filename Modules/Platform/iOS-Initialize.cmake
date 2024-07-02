include(Platform/Darwin-Initialize)

if(NOT _CMAKE_OSX_SYSROOT_PATH MATCHES "/(iPhoneOS|iPhoneSimulator|MacOSX)")
  message(FATAL_ERROR "${CMAKE_OSX_SYSROOT} is not an iOS SDK")
endif()

set(IOS 1)

set(_CMAKE_FEATURE_DETECTION_TARGET_TYPE STATIC_LIBRARY)
