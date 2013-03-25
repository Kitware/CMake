
project(BinaryDirectoryInInterface)

add_library(testTarget "${CMAKE_CURRENT_SOURCE_DIR}/empty.cpp")
target_include_directories(testTarget INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/foo")

install(TARGETS testTarget EXPORT testTargets
  DESTINATION lib
)

install(EXPORT testTargets DESTINATION lib/cmake)
