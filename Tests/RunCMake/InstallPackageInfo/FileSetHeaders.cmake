add_library(foo INTERFACE)

target_sources(foo
  INTERFACE
    FILE_SET no_genex
    TYPE HEADERS

  INTERFACE
    FILE_SET no_genex_dup
    TYPE HEADERS

  INTERFACE
    FILE_SET genex
    TYPE HEADERS

  INTERFACE
    FILE_SET genex_dup
    TYPE HEADERS
)

install(
  TARGETS foo
  EXPORT foo
  DESTINATION .

  FILE_SET no_genex
    DESTINATION no_genex

  FILE_SET no_genex_dup
    DESTINATION no_genex

  FILE_SET genex
    DESTINATION $<$<CONFIG:FAKE_CONFIG>:FAKE_DEST>genex

  FILE_SET genex_dup
    DESTINATION $<$<CONFIG:FAKE_CONFIG>:FAKE_DEST>genex
)
install(PACKAGE_INFO foo DESTINATION cps EXPORT foo)
