add_library(SourceGroupFileSetGenex INTERFACE)
target_sources(SourceGroupFileSetGenex PUBLIC FILE_SET HEADERS FILES "$<$<BOOL:TRUE>:iface.h>")
source_group("Header Files/SourceGroupFileSetGenex" FILES "$<$<BOOL:TRUE>:iface.h>")
