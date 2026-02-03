set(CMAKE_C_LINK_FLAGS ${pre}BADFLAG${obj})
add_library(LinkFlags_shared SHARED LinkFlagsLib.c)
add_executable(LinkFlags_exe LinkFlagsExe.c)
