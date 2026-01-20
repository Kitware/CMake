add_library(LinkFlags_shared SHARED LinkFlagsLib.c)
set_property(TARGET LinkFlags_shared PROPERTY LINK_FLAGS ${pre}BADFLAG_SHARED${obj})

add_library(LinkFlags_mod MODULE LinkFlagsLib.c)
set_property(TARGET LinkFlags_mod PROPERTY LINK_FLAGS ${pre}BADFLAG_MODULE${obj})

add_executable(LinkFlags_exe LinkFlagsExe.c)
set_property(TARGET LinkFlags_exe PROPERTY LINK_FLAGS ${pre}BADFLAG_EXECUTABLE${obj})
