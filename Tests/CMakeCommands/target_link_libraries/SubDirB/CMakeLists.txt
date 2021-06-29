add_executable(SubDirB SubDirB.c)

# Link to a target imported in this directory that would not normally
# be visible to the directory in which TopDir is defined.
target_link_libraries(TopDir PUBLIC debug SameNameImported optimized SameNameImported)
# Do the same through an interface library in the top.
target_link_libraries(TopDirInterface INTERFACE debug SameNameImported2 optimized SameNameImported2)

# Link to a list of targets imported in this directory that would not
# normally be visible to the directory in which TopDir is defined.
target_link_libraries(TopDir PUBLIC "$<1:SameNameImported;SameNameImported>")

# Link SubDirA to a target imported in this directory that has the same
# name as a target imported in SubDirA's directory.  We verify when
# compiling SubDirA that it sees our target and its own.
target_link_libraries(SubDirA PRIVATE SameNameImported)

# Import a target with the same name as a target imported in SubDirA.
# Distinguish this copy by having a unique usage requirement.
add_library(SameNameImported IMPORTED INTERFACE)
target_compile_definitions(SameNameImported INTERFACE DEF_SameNameImportedSubDirB)
add_library(SameNameImported2 INTERFACE IMPORTED)
target_compile_definitions(SameNameImported2 INTERFACE DEF_SameNameImported2SubDirB)
