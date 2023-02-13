enable_language(C)

set(CMAKE_XCODE_XCCONFIG "$<IF:$<CONFIG:Debug>,XcodeXCConfig.global.debug.xcconfig,XcodeXCConfig.global.release.xcconfig>")

add_library(somelib XcodeXCConfig.c)
target_compile_definitions(somelib PUBLIC "BUILD_DEBUG=$<IF:$<CONFIG:Debug>,1,0>")
set_target_properties(somelib PROPERTIES
  XCODE_XCCONFIG "$<IF:$<CONFIG:Debug>,XcodeXCConfig.target.debug.xcconfig,XcodeXCConfig.target.release.xcconfig>"
)
