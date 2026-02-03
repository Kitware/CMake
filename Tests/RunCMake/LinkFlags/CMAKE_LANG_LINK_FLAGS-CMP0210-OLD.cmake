cmake_policy(SET CMP0210 OLD)

set(CMAKE_C_LINK_FLAGS ${pre}BADFLAG${obj})

add_library(LinkFlags_shared SHARED LinkFlagsLib.c)
add_library(LinkFlags_mod MODULE LinkFlagsLib.c)
add_executable(LinkFlags_exe LinkFlagsExe.c)
