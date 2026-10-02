enable_language(C)

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME DefaultComponent)

add_library(default_component STATIC lib.c)
install(TARGETS default_component
  ARCHIVE DESTINATION lib
)

add_library(other_component STATIC neverbuild.c)
install(TARGETS other_component
  ARCHIVE DESTINATION lib
  COMPONENT Other
)

add_library(unit_tests STATIC neverbuild.c)
