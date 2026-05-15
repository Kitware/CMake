file(GLOB_RECURSE pdb_files LIST_DIRECTORIES false
  "${RunCMake_TEST_BINARY_DIR}/*.pdb")
foreach(pdb_file IN LISTS pdb_files)
  get_filename_component(pdb_name "${pdb_file}" NAME)
  if(pdb_name STREQUAL "SwiftPDBDwarf.pdb")
    string(APPEND RunCMake_TEST_FAILED
      "DWARF link unexpectedly produced ${pdb_file}\n")
  endif()
endforeach()
