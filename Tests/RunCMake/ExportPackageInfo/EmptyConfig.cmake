project(EmptyConfig CXX)

set(CMAKE_BUILD_TYPE "" CACHE STRING "" FORCE)
set(CMAKE_CONFIGURATION_TYPES "" CACHE STRING "" FORCE)

add_library(foo foo.cxx)

install(TARGETS foo EXPORT foo)
export(PACKAGE_INFO foo EXPORT foo)
