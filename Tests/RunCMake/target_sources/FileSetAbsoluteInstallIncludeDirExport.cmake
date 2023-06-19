enable_language(C)

# According to https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html#module:GNUInstallDirs
# relative CMAKE_INSTALL_<dir> are encouraged, but absolute path's are also allowed.
# Construct an absolute CMAKE_INSTALL_INCLUDEDIR.
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")

add_library(lib1)
target_sources(lib1
    PRIVATE lib1.c
    PUBLIC FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES h1.h)
# Expect install(TARGETS) to respect absolute CMAKE_INSTALL_INCLUDEDIR
# when installing the HEADERS.
# Must not prepend the CMAKE_INSTALL_PREFIX in the <pkg>-config.cmake.
install(TARGETS lib1 EXPORT lib1-config FILE_SET HEADERS)
install(EXPORT lib1-config NAMESPACE lib1:: DESTINATION share/lib1)
