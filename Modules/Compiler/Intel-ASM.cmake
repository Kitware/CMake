set(CMAKE_ASM_VERBOSE_FLAG "-v")

set(CMAKE_ASM_FLAGS_INIT "")
set(CMAKE_ASM_FLAGS_DEBUG_INIT "-g")
set(CMAKE_ASM_FLAGS_MINSIZEREL_INIT "-Os -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELEASE_INIT "-O3 -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")

if(UNIX)
  set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s;S)
else(UNIX)
  set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS asm)
endif(UNIX)
