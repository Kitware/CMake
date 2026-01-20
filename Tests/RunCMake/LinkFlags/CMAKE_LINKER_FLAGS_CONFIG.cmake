set(CMAKE_SHARED_LINKER_FLAGS_RELEASE ${pre}BADFLAG_SHARED_RELEASE${obj})
add_library(LinkFlags_shared SHARED LinkFlagsLib.c)

set(CMAKE_MODULE_LINKER_FLAGS_RELEASE ${pre}BADFLAG_MODULE_RELEASE${obj})
add_library(LinkFlags_mod MODULE LinkFlagsLib.c)

set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${pre}BADFLAG_EXECUTABLE_RELEASE${obj})
add_executable(LinkFlags_exe LinkFlagsExe.c)
