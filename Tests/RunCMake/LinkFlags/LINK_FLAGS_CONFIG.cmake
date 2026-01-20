add_library(LinkFlags_shared SHARED LinkFlagsLib.c)
set_property(TARGET LinkFlags_shared PROPERTY LINK_FLAGS_RELEASE ${pre}BADFLAG_SHARED_RELEASE${obj})

add_library(LinkFlags_mod MODULE LinkFlagsLib.c)
set_property(TARGET LinkFlags_mod PROPERTY LINK_FLAGS_RELEASE ${pre}BADFLAG_MODULE_RELEASE${obj})

add_executable(LinkFlags_exe LinkFlagsExe.c)
set_property(TARGET LinkFlags_exe PROPERTY LINK_FLAGS_RELEASE ${pre}BADFLAG_EXECUTABLE_RELEASE${obj})
