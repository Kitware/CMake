pdb-output-directory-target-context
-----------------------------------

* The :prop_tgt:`PDB_OUTPUT_DIRECTORY` and
  :prop_tgt:`COMPILE_PDB_OUTPUT_DIRECTORY` target properties, including their
  per-configuration variants, may now use generator expressions that refer
  to the target on which they are set, such as
  :genex:`$<TARGET_PROPERTY:prop>`.
