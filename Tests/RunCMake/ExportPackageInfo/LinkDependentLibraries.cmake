project(dyld CXX)

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(CMAKE_CONFIGURATION_TYPES "generic" CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE "generic" CACHE STRING "" FORCE)
endif()

add_library(private SHARED foo.cxx)
add_library(public SHARED foo.cxx)

target_link_libraries(public PRIVATE private)

install(TARGETS private public EXPORT dyld DESTINATION .)
export(PACKAGE_INFO dyld EXPORT dyld)
