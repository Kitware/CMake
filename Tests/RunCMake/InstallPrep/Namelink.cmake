enable_language(C)

add_library(namelink SHARED EXCLUDE_FROM_ALL lib.c)
set_target_properties(namelink PROPERTIES
  VERSION 1.0
  SOVERSION 1
)
install(TARGETS namelink
  LIBRARY DESTINATION lib
    COMPONENT Runtime
    NAMELINK_COMPONENT Namelink
)
