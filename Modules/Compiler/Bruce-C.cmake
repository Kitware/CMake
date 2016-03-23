# Bruce C Compiler ignores "-g" flag and optimization cannot be
# enabled here (it is implemented only for 8086 target).
set (CMAKE_C_FLAGS_INIT "-D__CLASSIC_C__")
set (CMAKE_C_FLAGS_DEBUG_INIT "-g")
set (CMAKE_C_FLAGS_MINSIZEREL_INIT "-DNDEBUG")
set (CMAKE_C_FLAGS_RELEASE_INIT "-DNDEBUG")
set (CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-g -DNDEBUG")
