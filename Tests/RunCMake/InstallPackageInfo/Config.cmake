project(Config CXX)

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(CMAKE_CONFIGURATION_TYPES "FooConfig;BarConfig" CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE "TestConfig" CACHE STRING "" FORCE)
endif()

add_library(foo foo.cxx)

install(TARGETS foo EXPORT foo)
install(PACKAGE_INFO foo DESTINATION cps EXPORT foo)
