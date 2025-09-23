project(TestLibrary C)

add_library(liba SHARED liba.c)
add_library(libb SHARED libb.c)

target_link_libraries(libb PUBLIC liba)

install(TARGETS liba EXPORT liba DESTINATION lib)
export(EXPORT liba PACKAGE_INFO liba)

install(TARGETS libb EXPORT libb DESTINATION lib)
export(EXPORT libb PACKAGE_INFO libb)
