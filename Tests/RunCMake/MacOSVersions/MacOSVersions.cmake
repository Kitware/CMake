enable_language(C)

add_library(foo SHARED foo.c)
set_target_properties(foo PROPERTIES
  VERSION 1.0
  SOVERSION 1
  OSX_COMPATIBILITY_VERSION 2.1.0
  OSX_CURRENT_VERSION 3.2.1
  )
