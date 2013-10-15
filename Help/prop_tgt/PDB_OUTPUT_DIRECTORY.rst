PDB_OUTPUT_DIRECTORY
--------------------

Output directory for MS debug symbols .pdb file from linker.

This property specifies the directory into which the MS debug symbols
will be placed by the linker.  This property is initialized by the
value of the variable CMAKE_PDB_OUTPUT_DIRECTORY if it is set when a
target is created.

This property is not implemented by the Visual Studio 6 generator.
