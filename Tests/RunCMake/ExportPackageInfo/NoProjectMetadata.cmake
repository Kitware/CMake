project(foo
  VERSION 1.2.3
  COMPAT_VERSION 1.1.0
  DESCRIPTION "Sample package"
  HOMEPAGE_URL "https://www.example.com/package/foo"
  )

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

export(
  PACKAGE_INFO foo
  EXPORT foo
  NO_PROJECT_METADATA
  )
