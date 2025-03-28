find_package(
  test REQUIRED CONFIG
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
  )

add_library(libb INTERFACE)
add_library(libc INTERFACE)
add_library(libd INTERFACE)

add_library(foo ALIAS libb)
add_library(bar ALIAS libc)

target_link_libraries(libd INTERFACE test::liba foo bar)

install(TARGETS libb EXPORT foo DESTINATION .)
install(PACKAGE_INFO foo DESTINATION cps EXPORT foo)

install(TARGETS libc libd EXPORT bar DESTINATION .)
install(PACKAGE_INFO bar DESTINATION cps EXPORT bar)
