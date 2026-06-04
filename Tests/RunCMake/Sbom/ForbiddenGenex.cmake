project(ForbiddenGenex C)

add_library(foo ${CMAKE_CURRENT_LIST_DIR}/test.c)

find_package(
  bar 1.3.4 REQUIRED
  NO_DEFAULT_PATH
  PATHS ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(foo INTERFACE
  $<TARGET_PROPERTY:foo>
)
install(TARGETS foo EXPORT foo DESTINATION .)
