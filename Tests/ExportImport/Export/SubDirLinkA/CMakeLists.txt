add_library(SubDirLinkAImported IMPORTED INTERFACE)
target_compile_definitions(SubDirLinkAImported INTERFACE DEF_SubDirLinkAImportedForExport)

target_link_libraries(TopDirLib PUBLIC
  debug "$<1:$<TARGET_NAME:SubDirLinkAImported>;$<TARGET_NAME:SubDirLinkAImported>>"
  optimized "$<1:$<TARGET_NAME:SubDirLinkAImported>;$<TARGET_NAME:SubDirLinkAImported>>"
)

add_library(SubDirLinkA STATIC SubDirLinkA.c)
