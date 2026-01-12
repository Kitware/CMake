project(foo
  VERSION 1.2.3
  COMPAT_VERSION 1.1.0
  SPDX_LICENSE "BSD-3-Clause"
  DESCRIPTION "Sample package"
  HOMEPAGE_URL "https://www.example.com/package/foo"
  )

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Test inheriting from project matching package name.
export(
  PACKAGE_INFO foo
  EXPORT foo
  )

# Test inheriting from a specified project.
export(
  PACKAGE_INFO test1
  EXPORT foo
  PROJECT foo
  )

# Test that inheriting doesn't override explicitly specified metadata.
export(
  PACKAGE_INFO test2
  EXPORT foo
  PROJECT foo
  VERSION 1.4.7
  LICENSE "Apache-2.0"
  DESCRIPTION "Don't inherit"
  HOMEPAGE_URL "https://www.example.com/package/bar"
  )
