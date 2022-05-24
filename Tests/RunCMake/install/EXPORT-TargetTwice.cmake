enable_language(C)

add_library(foo STATIC empty.c)

install(TARGETS foo
  EXPORT pkg1
  ARCHIVE  DESTINATION pkg1/lib
  INCLUDES DESTINATION pkg1/inc
  )
install(EXPORT pkg1 DESTINATION pkg1)

install(TARGETS foo
  EXPORT pkg2
  ARCHIVE  DESTINATION pkg2/lib
  INCLUDES DESTINATION pkg2/inc
  )
install(EXPORT pkg2 DESTINATION pkg2)
