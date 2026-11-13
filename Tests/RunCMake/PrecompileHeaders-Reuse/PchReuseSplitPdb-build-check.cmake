function(check_pdb pdb created)
  get_filename_component(dir "${pdb}" DIRECTORY)
  get_filename_component(name "${pdb}" NAME)
  if(EXISTS "${RunCMake_TEST_BINARY_DIR}/${pdb}" OR
     EXISTS "${RunCMake_TEST_BINARY_DIR}/${dir}/Debug/${name}")
    if(NOT created)
      string(APPEND RunCMake_TEST_FAILED "Compiler PDB '${pdb}' created.\n")
    endif()
  elseif(created)
    string(APPEND RunCMake_TEST_FAILED "Compiler PDB '${pdb}' not created.\n")
  endif()
  set(RunCMake_TEST_FAILED "${RunCMake_TEST_FAILED}" PARENT_SCOPE)
endfunction()

foreach(name IN ITEMS reuse_old reuse_default reuse_named reuse_legacy)
  check_pdb(${name}/creator.pdb 1)
endforeach()

# With CMP0235 unset, the copy is the only compiler PDB.  Older MSVC keeps
# that layout with CMP0235 NEW too.
set(own_pdb 1)
if(MSVC_VERSION LESS 1916)
  set(own_pdb 0)
endif()
foreach(pdb IN ITEMS
    reuse_default/reuse_default.pdb
    reuse_named/named.pdb
    reuse_legacy/reuse_legacy.pdb
    )
  check_pdb(${pdb} ${own_pdb})
endforeach()
foreach(pdb IN ITEMS reuse_old/reuse_old.pdb reuse_old/named_old.pdb)
  check_pdb(${pdb} 0)
endforeach()
