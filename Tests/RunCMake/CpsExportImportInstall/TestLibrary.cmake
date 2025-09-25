project(TestLibrary C)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../install")

add_library(liba SHARED liba.c)
add_library(libb SHARED libb.c)

target_link_libraries(libb PUBLIC liba)

install(TARGETS liba EXPORT liba DESTINATION lib)
install(PACKAGE_INFO liba DESTINATION cps EXPORT liba)

install(TARGETS libb EXPORT libb DESTINATION lib)
install(PACKAGE_INFO libb DESTINATION cps EXPORT libb)
