add_library(SourceGroupFileSet INTERFACE)
target_sources(SourceGroupFileSet PUBLIC FILE_SET HEADERS FILES iface.h)
source_group("Header Files/SourceGroupFileSet" FILES iface.h)
