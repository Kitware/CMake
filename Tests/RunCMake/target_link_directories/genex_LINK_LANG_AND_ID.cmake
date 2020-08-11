
enable_language(C)
enable_language(CXX)

set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES FALSE)

add_library(LinkDirs_interface INTERFACE)
target_link_directories (LinkDirs_interface INTERFACE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:/DIR_C_INTERFACE>
                                                      $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:/DIR_CXX_INTERFACE>
                                                      $<$<LINK_LANG_AND_ID:C,BADID>:/DIR_C_BADID_INTERFACE>
                                                      $<$<LINK_LANG_AND_ID:CXX,BADID>:/DIR_CXX_BADID_INTERFACE>)

add_executable(LinkDirs_exe_interface exe.c)
target_link_libraries (LinkDirs_exe_interface PRIVATE LinkDirs_interface)

add_executable(LinkDirs_exe_c exe.c)
target_link_options (LinkDirs_exe_c PRIVATE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:/DIR_C_EXE>
                                            $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:/DIR_CXX_EXE>
                                            $<$<LINK_LANG_AND_ID:C,BADID>:/DIR_C_BADID_EXE>
                                            $<$<LINK_LANG_AND_ID:CXX,BADID>:/DIR_CXX_BADID_EXE>)

add_executable(LinkDirs_exe_cxx exe.c)
target_link_directories (LinkDirs_exe_cxx PRIVATE $<$<LINK_LANG_AND_ID:C,${CMAKE_C_COMPILER_ID}>:/DIR_C_EXE>
                                                  $<$<LINK_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:/DIR_CXX_EXE>
                                                  $<$<LINK_LANG_AND_ID:C,BADID>:/DIR_C_BADID_EXE>
                                                  $<$<LINK_LANG_AND_ID:CXX,BADID>:/DIR_CXX_BADID_EXE>)
set_property (TARGET LinkDirs_exe_cxx PROPERTY LINKER_LANGUAGE CXX)
