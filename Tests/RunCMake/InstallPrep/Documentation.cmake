enable_language(C)

install(FILES lib.c
  DESTINATION share/install-prep
  COMPONENT Documentation
)

add_library(unit_tests STATIC neverbuild.c)
