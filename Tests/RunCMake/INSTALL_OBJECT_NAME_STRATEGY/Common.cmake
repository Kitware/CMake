add_library(objlib OBJECT test.c subdir/inner_test.c)
install(TARGETS objlib EXPORT exp OBJECTS DESTINATION "lib/objlib")
install(EXPORT exp DESTINATION lib/cmake/IONS FILE ions-config.cmake NAMESPACE IONS::)
