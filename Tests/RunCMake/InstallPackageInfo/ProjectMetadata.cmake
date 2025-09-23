project(foo
  VERSION 1.2.3
  COMPAT_VERSION 1.1.0
  DESCRIPTION "Sample package"
  HOMEPAGE_URL "https://www.example.com/package/foo"
  )

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Test inheriting from project matching package name.
install(
  PACKAGE_INFO foo
  DESTINATION cps
  EXPORT foo
  )

# Test inheriting from a specified project.
install(
  PACKAGE_INFO test1
  DESTINATION cps
  EXPORT foo
  PROJECT foo
  )

# Test that inheriting doesn't override explicitly specified metadata.
install(
  PACKAGE_INFO test2
  DESTINATION cps
  EXPORT foo
  PROJECT foo
  VERSION 1.4.7
  DESCRIPTION "Don't inherit"
  HOMEPAGE_URL "https://www.example.com/package/bar"
  )
