enable_language(C)

add_library(nested STATIC lib.c)
install(TARGETS nested
  ARCHIVE DESTINATION lib
  COMPONENT Nested/Runtime_1.2+dbg
)

add_library(unit_tests STATIC neverbuild.c)
