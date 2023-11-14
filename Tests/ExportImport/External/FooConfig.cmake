if(NOT TARGET Foo::Foo)
  add_library(Foo::Foo INTERFACE IMPORTED)
endif()
