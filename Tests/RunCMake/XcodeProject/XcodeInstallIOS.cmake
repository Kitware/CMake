cmake_minimum_required(VERSION 3.3)

project(XcodeInstallIOS)

set(XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

add_library(foo STATIC foo.cpp)
install(TARGETS foo ARCHIVE DESTINATION lib)
