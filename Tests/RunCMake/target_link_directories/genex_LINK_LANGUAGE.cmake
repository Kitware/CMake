
enable_language(C)
enable_language(CXX)

set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)

add_library(LinkDirs_interface INTERFACE)
target_link_directories (LinkDirs_interface INTERFACE "$<$<LINK_LANGUAGE:C>:/DIR_C_INTERFACE>"
                                                      "$<$<LINK_LANGUAGE:CXX>:/DIR_CXX_INTERFACE>")

add_executable(LinkDirs_exe_interface exe.c)
target_link_libraries (LinkDirs_exe_interface PRIVATE LinkDirs_interface)

add_executable(LinkDirs_exe_c exe.c)
target_link_directories (LinkDirs_exe_c PRIVATE "$<$<LINK_LANGUAGE:C>:/DIR_C_EXE>"
                                                "$<$<LINK_LANGUAGE:CXX>:/DIR_CXX_EXE>")

add_executable(LinkDirs_exe_cxx exe.c)
target_link_directories (LinkDirs_exe_cxx PRIVATE "$<$<LINK_LANGUAGE:C>:/DIR_C_EXE>"
                                                 "$<$<LINK_LANGUAGE:CXX>:/DIR_CXX_EXE>")
set_property (TARGET LinkDirs_exe_cxx PROPERTY LINKER_LANGUAGE CXX)
