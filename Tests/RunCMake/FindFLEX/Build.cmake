find_package(FLEX REQUIRED)

flex_target(Scanner scanner.l scanner.c)
flex_target(Nested scanner.l src/nested.c
  DEFINES_FILE include/nested/nested.h
  )

add_custom_target(scanners ALL
  DEPENDS ${FLEX_Scanner_OUTPUTS} ${FLEX_Nested_OUTPUTS})
