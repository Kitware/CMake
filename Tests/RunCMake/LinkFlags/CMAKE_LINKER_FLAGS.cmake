set(CMAKE_SHARED_LINKER_FLAGS ${pre}BADFLAG_SHARED${obj})
add_library(LinkFlags_shared SHARED LinkFlagsLib.c)

set(CMAKE_MODULE_LINKER_FLAGS ${pre}BADFLAG_MODULE${obj})
add_library(LinkFlags_mod MODULE LinkFlagsLib.c)

set(CMAKE_EXE_LINKER_FLAGS ${pre}BADFLAG_EXECUTABLE${obj})
add_executable(LinkFlags_exe LinkFlagsExe.c)
