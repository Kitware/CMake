set(CMAKE_ASM_VERBOSE_FLAG "-V")

# -qthreaded     = Ensures that all optimizations will be thread-safe
# -qhalt=e       = Halt on error messages (rather than just severe errors)
set(CMAKE_ASM_FLAGS_INIT "-qthreaded -qhalt=e -qsourcetype=assembler")

set(CMAKE_ASM_FLAGS_DEBUG_INIT "-g")
set(CMAKE_ASM_FLAGS_RELEASE_INIT "-O -DNDEBUG")
set(CMAKE_ASM_FLAGS_MINSIZEREL_INIT "-O -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO_INIT "-g -DNDEBUG")

set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS s )
