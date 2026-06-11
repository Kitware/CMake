get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(CMAKE_CONFIGURATION_TYPES "BarConfig;BazConfig" CACHE STRING "" FORCE)
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "BarConfig" CACHE STRING "" FORCE)
  endif()
endif()

project(Genex C)
add_library(foo INTERFACE)

find_package(
  bar 1.3.4 REQUIRED
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

find_package(
  baz 1.3.4 REQUIRED
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(foo INTERFACE $<$<CONFIG:BarConfig>:bar::bar>)
target_link_libraries(foo INTERFACE $<$<CONFIG:BazConfig>:baz>)

install(TARGETS foo EXPORT foo)
