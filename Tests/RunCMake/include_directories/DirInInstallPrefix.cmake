enable_language(CXX)
add_library(testTarget empty.cpp)
target_include_directories(testTarget INTERFACE "${CMAKE_INSTALL_PREFIX}/dir")

install(TARGETS testTarget EXPORT testTargets
  DESTINATION lib
)

install(EXPORT testTargets DESTINATION lib/cmake)
