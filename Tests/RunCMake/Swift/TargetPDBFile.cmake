cmake_policy(SET CMP0157 NEW)

enable_language(Swift)

add_executable(SwiftPDB E.swift)
set_property(TARGET SwiftPDB PROPERTY PDB_NAME SwiftPDBCustom)
set_property(TARGET SwiftPDB PROPERTY PDB_OUTPUT_DIRECTORY
  "$<1:${CMAKE_CURRENT_BINARY_DIR}/pdb>")

install(FILES "$<TARGET_PDB_FILE:SwiftPDB>"
  DESTINATION bin
  OPTIONAL)
