enable_language(C)

add_library(hidden STATIC lib.c)
install(TARGETS hidden
  ARCHIVE DESTINATION lib
  COMPONENT Hidden
  EXCLUDE_FROM_ALL
)

add_library(unit_tests STATIC neverbuild.c)
