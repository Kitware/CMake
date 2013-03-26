
project(RelativePathInInterface)

add_library(testTarget "${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp")
set_property(TARGET testTarget PROPERTY INTERFACE_INCLUDE_DIRECTORIES "foo")

install(TARGETS testTarget EXPORT testTargets
  DESTINATION lib
)

install(EXPORT testTargets DESTINATION lib/cmake)
