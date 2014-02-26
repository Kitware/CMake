msvc-compiler-pdb-files
-----------------------

* New :prop_tgt:`COMPILE_PDB_NAME` and
  :prop_tgt:`COMPILE_PDB_OUTPUT_DIRECTORY` target properties
  were introduced to specify the MSVC compiler program database
  file location (``cl /Fd``).  This complements the existing
  :prop_tgt:`PDB_NAME` and :prop_tgt:`PDB_OUTPUT_DIRECTORY`
  target properties that specify the linker program database
  file location (``link /pdb``).
