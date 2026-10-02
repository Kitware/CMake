add_library(development STATIC ../lib.c)
install(TARGETS development
  ARCHIVE DESTINATION lib
  COMPONENT Development
)
