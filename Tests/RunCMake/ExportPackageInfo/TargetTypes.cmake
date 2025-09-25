project(TargetTypes CXX)

add_library(foo-static STATIC foo.cxx)
add_library(foo-shared SHARED foo.cxx)
add_library(foo-module MODULE foo.cxx)
add_library(bar INTERFACE)
add_executable(test test.cxx)

install(
  TARGETS
    foo-static
    foo-shared
    foo-module
    bar
    test
  EXPORT foo
  DESTINATION .
  )

export(EXPORT foo PACKAGE_INFO foo)
