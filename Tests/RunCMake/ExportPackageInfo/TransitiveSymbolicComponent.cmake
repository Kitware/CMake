cmake_minimum_required(VERSION 4.0)

add_library(bar INTERFACE)

find_package(Symbolic REQUIRED CONFIG
  COMPONENTS foo test
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(bar INTERFACE Symbolic::test)

install(TARGETS bar EXPORT bar DESTINATION .)
export(PACKAGE_INFO bar EXPORT bar)
