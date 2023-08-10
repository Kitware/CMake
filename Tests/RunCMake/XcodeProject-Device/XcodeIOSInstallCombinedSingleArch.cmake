enable_language(CXX)

set(iphoneos_arch armv7)
if(XCODE_VERSION VERSION_GREATER_EQUAL 14)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 16)
  set(iphoneos_arch arm64)
elseif(XCODE_VERSION VERSION_GREATER_EQUAL 9)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10)
endif()

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf")

add_library(foo SHARED foo.cpp)
install(TARGETS foo DESTINATION lib)

set_target_properties(
  foo
  PROPERTIES
  XCODE_ATTRIBUTE_ARCHS[sdk=iphoneos*] ${iphoneos_arch}
  XCODE_ATTRIBUTE_VALID_ARCHS[sdk=iphoneos*] ${iphoneos_arch}
  XCODE_ATTRIBUTE_ARCHS[sdk=iphonesimulator*] ""
  XCODE_ATTRIBUTE_VALID_ARCHS[sdk=iphonesimulator*] ""
)
