project(foo VERSION 1.2.3 COMPAT_VERSION 1.1.0)

add_library(foo INTERFACE)
install(TARGETS foo EXPORT foo DESTINATION .)

# Test inheriting from project matching package name.
export(
  EXPORT foo
  PACKAGE_INFO foo
  )

# Test inheriting from a specified project.
export(
  EXPORT foo
  PROJECT foo
  PACKAGE_INFO test1
  )

# Test that inheriting doesn't override explicitly specified metadata.
export(
  EXPORT foo
  PROJECT foo
  PACKAGE_INFO test2
  VERSION 1.4.7
  )
