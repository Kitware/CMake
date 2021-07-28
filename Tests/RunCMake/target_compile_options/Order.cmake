get_property (isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
  set(CMAKE_CONFIGURATION_TYPES "Custom" CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE "Custom" CACHE STRING "" FORCE)
endif()
enable_language(C)

string(APPEND CMAKE_C_FLAGS " -w -O")
set(CMAKE_C_FLAGS_CUSTOM "-O0")

add_executable(order order.c)
set_property(TARGET order APPEND PROPERTY COMPILE_OPTIONS -O1)

add_library(iface INTERFACE)
set_property(TARGET iface APPEND PROPERTY INTERFACE_COMPILE_OPTIONS -O2)
target_link_libraries(order PRIVATE iface)

set_property(SOURCE order.c PROPERTY COMPILE_OPTIONS -O3)
