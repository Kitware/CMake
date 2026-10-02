enable_language(C)

add_library(debug_only STATIC EXCLUDE_FROM_ALL lib.c)
install(TARGETS debug_only
  ARCHIVE DESTINATION lib
  COMPONENT Configured
  CONFIGURATIONS Debug
)

add_library(release_only STATIC EXCLUDE_FROM_ALL lib.c)
install(TARGETS release_only
  ARCHIVE DESTINATION lib
  COMPONENT Configured
  CONFIGURATIONS Release
)

add_library(unit_tests STATIC neverbuild.c)
