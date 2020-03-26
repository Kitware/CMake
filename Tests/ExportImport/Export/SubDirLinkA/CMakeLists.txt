add_library(SubDirLinkAImported IMPORTED INTERFACE)
target_compile_definitions(SubDirLinkAImported INTERFACE DEF_SubDirLinkAImportedForExport)

target_link_libraries(TopDirLib PUBLIC debug "$<1:SubDirLinkAImported;SubDirLinkAImported>" optimized "$<1:SubDirLinkAImported;SubDirLinkAImported>")

add_library(SubDirLinkA STATIC SubDirLinkA.c)
