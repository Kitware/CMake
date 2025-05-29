project(foo VERSION 1.2.3 COMPAT_VERSION 1.1.0)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

export(
  EXPORT foo
  PACKAGE_INFO foo
  NO_PROJECT_METADATA
  )
