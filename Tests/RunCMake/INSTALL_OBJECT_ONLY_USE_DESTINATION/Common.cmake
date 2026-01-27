add_library(objlib OBJECT lib.c)
install(TARGETS objlib
  EXPORT exp
  DESTINATION lib/objlib)
install(EXPORT exp DESTINATION lib/cmake/IOOUD FILE iooud-config.cmake NAMESPACE IOOUD::)
