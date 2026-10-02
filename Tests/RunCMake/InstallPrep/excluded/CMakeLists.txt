add_library(excluded_subdirectory STATIC ../neverbuild.c)

install(TARGETS excluded_subdirectory
  ARCHIVE DESTINATION lib
  COMPONENT ExcludedSubdirectory
)
