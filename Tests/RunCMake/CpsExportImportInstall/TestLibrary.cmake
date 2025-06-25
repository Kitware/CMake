project(TestLibrary C)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../install")

add_library(liba SHARED)
target_sources(liba
  PRIVATE
    liba/liba.c
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
      liba
    FILES
      liba/liba.h
)

add_library(libb SHARED)
target_sources(libb
  PRIVATE
    libb/libb.c
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
      libb
    FILES
      libb/libb.h
)

target_link_libraries(libb PUBLIC liba)

install(TARGETS liba EXPORT liba FILE_SET HEADERS)
install(PACKAGE_INFO liba DESTINATION cps EXPORT liba)

install(TARGETS libb EXPORT libb FILE_SET HEADERS)
install(PACKAGE_INFO libb DESTINATION cps EXPORT libb)
