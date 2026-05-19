cmake_policy(SET CMP0157 NEW)

enable_language(Swift)

add_executable(SwiftPDBDwarf E.swift)
set_property(TARGET SwiftPDBDwarf PROPERTY LINKER_TYPE LLD)
target_compile_options(SwiftPDBDwarf PRIVATE -g -debug-info-format=dwarf)
target_link_options(SwiftPDBDwarf PRIVATE LINKER:-debug:dwarf)

install(FILES "$<TARGET_PDB_FILE:SwiftPDBDwarf>"
  DESTINATION bin
  OPTIONAL)
