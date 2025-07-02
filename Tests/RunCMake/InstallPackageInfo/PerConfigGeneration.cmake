project(PerConfigGeneration CXX)

set(config ${CMAKE_BUILD_TYPE})
if(NOT config)
  set(config noconfig)
endif()

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/build_type.cmake" "set(config \"${config}\")")

add_library(foo foo.cxx)

install(TARGETS foo EXPORT foo)
install(PACKAGE_INFO foo DESTINATION cps EXPORT foo)
