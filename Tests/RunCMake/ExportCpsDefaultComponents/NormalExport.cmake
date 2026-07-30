find_package(
  foo REQUIRED CONFIG
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
  )

add_library(bar INTERFACE)
target_link_libraries(bar INTERFACE foo)

install(TARGETS bar EXPORT bar)
export(EXPORT bar)
export(PACKAGE_INFO bar EXPORT bar)
