add_library(SubDirC STATIC SubDirC.c)

add_library(SubDirC1 INTERFACE)
target_compile_definitions(SubDirC1 INTERFACE DEF_SubDirC1)

add_library(SubDirC2 INTERFACE)
target_compile_definitions(SubDirC2 INTERFACE DEF_SubDirC2)

target_link_libraries(SubDirC PRIVATE SubDirC1)
