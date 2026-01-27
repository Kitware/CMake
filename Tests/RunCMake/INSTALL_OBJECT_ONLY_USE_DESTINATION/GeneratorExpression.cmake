set(CMAKE_INSTALL_OBJECT_ONLY_USE_DESTINATION 1)
add_library(objlib OBJECT lib.c)
install(TARGETS objlib
  EXPORT exp
  DESTINATION "lib/$<TARGET_NAME:objlib>/$<CONFIG>")
install(EXPORT exp DESTINATION lib/cmake/IOOUD FILE iooud-config.cmake NAMESPACE IOOUD::)
