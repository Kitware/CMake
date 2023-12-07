add_executable(SourceGroups2 main.c
                             qux.c subsub/qax.c)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" ) #PREFIX TheSubDir2 )
