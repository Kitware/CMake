project(Framework)

add_library(foo SHARED
  foo.cxx
  foo.h
  foo2.h
  fooPublic.h
  fooPrivate.h
  fooNeither.h
  fooBoth.h
  test.lua
)
set_target_properties(foo PROPERTIES
  FRAMEWORK TRUE
  FRAMEWORK_VERSION ver3
)
# fooNeither.h is marked neither public nor private...
# fooBoth.h is marked both public and private... (private wins...)
set_source_files_properties(foo.h foo2.h fooPublic.h fooBoth.h PROPERTIES
  FRAMEWORK_PUBLIC_HEADER TRUE
)
set_source_files_properties(fooPrivate.h fooBoth.h PROPERTIES
  FRAMEWORK_PRIVATE_HEADER TRUE
)
set_source_files_properties(test.lua PROPERTIES
  FRAMEWORK_RESOURCE TRUE
)
add_executable(bar bar.cxx)
target_link_libraries(bar foo)


# Make a static library and apply the framework properties to it to verify
# that everything still builds correctly, but it will not actually produce
# a framework... The framework properties only apply when the library type
# is SHARED.
#
add_library(fooStatic STATIC
  foo.cxx
  foo.h
  foo2.h
  fooPublic.h
  fooPrivate.h
  fooNeither.h
  fooBoth.h
  test.lua
)
set_target_properties(fooStatic PROPERTIES
  FRAMEWORK TRUE
  FRAMEWORK_VERSION none
)
add_executable(barStatic bar.cxx)
target_link_libraries(barStatic fooStatic)
