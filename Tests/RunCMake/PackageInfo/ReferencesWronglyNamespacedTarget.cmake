find_package(
  broken REQUIRED CONFIG
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
  )

add_library(foo INTERFACE)
target_link_libraries(foo INTERFACE wrong::lib)

install(TARGETS foo EXPORT foo DESTINATION .)
install(PACKAGE_INFO foo EXPORT foo)
