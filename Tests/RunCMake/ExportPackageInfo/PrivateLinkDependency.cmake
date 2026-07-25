project(PrivateLinkDependency CXX)

get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(CMAKE_CONFIGURATION_TYPES "generic" CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE "generic" CACHE STRING "" FORCE)
endif()

find_package(
  libdep REQUIRED CONFIG
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

add_library(mylib SHARED foo.cxx)
target_link_libraries(mylib PRIVATE libdep::libdep)

install(TARGETS mylib EXPORT mylib DESTINATION .)
export(PACKAGE_INFO mylib EXPORT mylib)
