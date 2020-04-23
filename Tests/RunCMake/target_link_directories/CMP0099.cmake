
enable_language(C)

set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)

add_library(LinkDirs_interface INTERFACE)
target_link_directories (LinkDirs_interface INTERFACE "/DIR_INTERFACE")

add_library(LinkDirs_static STATIC lib.c)
target_link_libraries (LinkDirs_static PRIVATE LinkDirs_interface)

add_executable(LinkDirs_exe exe.c)
target_link_libraries (LinkDirs_exe PRIVATE LinkDirs_static)
