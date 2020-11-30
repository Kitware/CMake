enable_language(C)
add_library(foo SHARED empty.c)
install(TARGETS foo EXPORT fooExport
  RUNTIME DESTINATION bin
  LIBRARY
     DESTINATION lib
     COMPONENT runtime
     NAMELINK_SKIP
)
install(TARGETS foo EXPORT fooExport
  LIBRARY
     DESTINATION lib
     COMPONENT development
     NAMELINK_ONLY
)
install(EXPORT fooExport
    DESTINATION "lib/cmake/"
    FILE "foo.cmake"
)
