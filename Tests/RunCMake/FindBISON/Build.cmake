find_package(BISON REQUIRED)

bison_target(Parser parser.y parser.c)
bison_target(Nested parser.y src/nested.c
  DEFINES_FILE include/nested/nested.h
  VERBOSE REPORT_FILE reports/nested.output
  )

add_custom_target(parsers ALL
  DEPENDS ${BISON_Parser_OUTPUTS} ${BISON_Nested_OUTPUTS})
