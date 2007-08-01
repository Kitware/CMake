project(Framework)

add_library(foo SHARED foo.cxx foo.h)
set_target_properties(foo PROPERTIES
  FRAMEWORK TRUE
  FRAMEWORK_PUBLIC_HEADERS "foo.h;foo2.h"
  FRAMEWORK_VERSION ver2
  FRAMEWORK_RESOURCES "test.lua"
)
add_executable(bar bar.cxx)
target_link_libraries(bar foo)


# Make a static library and apply the framework properties to it to verify
# that everything still builds correctly, but it will not actually produce
# a framework... The framework properties only apply when the library type
# is SHARED.
#
add_library(fooStatic STATIC foo.cxx foo.h)
set_target_properties(fooStatic PROPERTIES
  FRAMEWORK TRUE
  FRAMEWORK_PUBLIC_HEADERS "foo.h;foo2.h"
  FRAMEWORK_VERSION ver2
  FRAMEWORK_RESOURCES "test.lua"
)
add_executable(barStatic bar.cxx)
target_link_libraries(barStatic fooStatic)
