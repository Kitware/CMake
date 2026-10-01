include(common.cmake)
enable_language(C)

set(CMAKE_VERBOSE_MAKEFILE TRUE)

set_property(GLOBAL PROPERTY
  CMAKE_COMPILE_WRAPPER_EXAMPLE_1
    definitely-not-a-command
)
set_property(GLOBAL PROPERTY
  CMAKE_LINK_WRAPPER_EXAMPLE_1
    definitely-not-a-command
)

add_executable(main main.c)
set_target_properties(main PROPERTIES
  # Use the $<0:...> genexp to validate (a) genexp support and (b) handling of
  # no wrappers.
  COMPILE_WRAPPERS "$<0:EXAMPLE_1>"
  LINK_WRAPPERS "$<0:EXAMPLE_1>"
)
