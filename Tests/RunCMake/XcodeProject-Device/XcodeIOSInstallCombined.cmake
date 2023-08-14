enable_language(CXX)

set(maybe_armv7 armv7)
set(maybe_i386 i386)
if(XCODE_VERSION VERSION_GREATER_EQUAL 14)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 16)
  set(maybe_armv7 "")
  set(maybe_i386 "")
elseif(XCODE_VERSION VERSION_GREATER_EQUAL 9)
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10)
endif()

if(NOT IOS)
  message(FATAL_ERROR "IOS variable is not set")
endif()

set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")
set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf")
set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

set(CMAKE_OSX_ARCHITECTURES ${maybe_armv7} arm64 ${maybe_i386} x86_64)

add_executable(foo_app MACOSX_BUNDLE main.cpp)
install(TARGETS foo_app BUNDLE DESTINATION bin)

add_library(foo_static STATIC foo.cpp)
install(TARGETS foo_static ARCHIVE DESTINATION lib)

add_library(foo_shared SHARED foo.cpp)
install(TARGETS foo_shared LIBRARY DESTINATION lib)

add_library(foo_bundle MODULE foo.cpp)
set_target_properties(foo_bundle PROPERTIES BUNDLE TRUE)
install(TARGETS foo_bundle LIBRARY DESTINATION lib)

add_library(foo_framework SHARED foo.cpp)
set_target_properties(foo_framework PROPERTIES FRAMEWORK TRUE)
install(TARGETS foo_framework FRAMEWORK DESTINATION lib)
