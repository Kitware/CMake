add_library(foo INTERFACE)

# Primarily this tests for de-duplication of the BASE_DIRS, ensuring DESTINATION
# genex have no effect on build exports is a bonus covering a very unlikely bug

target_sources(foo
  INTERFACE
    FILE_SET no_genex
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/foo

  INTERFACE
    FILE_SET genex
    TYPE HEADERS
    BASE_DIRS ${CMAKE_CURRENT_LIST_DIR}/foo
)

install(
  TARGETS foo
  EXPORT foo
  DESTINATION .

  FILE_SET no_genex
    DESTINATION no_genex

  FILE_SET genex
    DESTINATION $<$<CONFIG:FAKE_CONFIG>:FAKE_DEST>genex
)
export(PACKAGE_INFO foo EXPORT foo)
