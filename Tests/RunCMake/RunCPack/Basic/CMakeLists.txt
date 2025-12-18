cmake_minimum_required(VERSION 4.2)
project(Basic LANGUAGES NONE VERSION "1.2.3")

install(FILES basic.txt DESTINATION share/doc/basic)

set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
set(CPACK_DEBIAN_COMPRESSION_LEVEL "4")

set(CPACK_PACKAGE_CONTACT "somebody@cmake.org")
include(CPack)
