enable_language(C)
add_library(foo SHARED empty.c)
install(TARGETS foo EXPORT fooExport
  RUNTIME DESTINATION bin
  LIBRARY
     DESTINATION lib
     NAMELINK_ONLY
)
install(EXPORT fooExport
    DESTINATION "lib/cmake/"
    FILE "foo.cmake"
)
