enable_language(C)
add_executable(empty1 empty.c)
set_property(TARGET empty1 PROPERTY PDB_NAME $<TARGET_PDB_FILE_NAME:empty1>)

add_executable(empty2 empty.c)
set_property(TARGET empty2 PROPERTY OUTPUT_NAME $<TARGET_PDB_FILE_BASE_NAME:empty2>)
