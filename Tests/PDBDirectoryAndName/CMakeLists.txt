cmake_minimum_required(VERSION 2.8)
project(PDBDirectoryAndName C)

# Make sure the proper compiler is in use.
if(NOT MSVC AND NOT "${CMAKE_C_COMPILER_ID}" MATCHES "^(Intel)$")
  message(FATAL_ERROR "The PDBDirectoryAndName test works only with MSVC or Intel")
endif()

set(my_targets "")

add_library(mylibA SHARED mylibA.c)
set_target_properties(mylibA PROPERTIES
    PDB_NAME "mylibA_Special"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mylibA_PDB"
)
list(APPEND my_targets mylibA)

add_library(mylibB STATIC mylibB.c)
set_target_properties(mylibB PROPERTIES
    PDB_NAME "mylibB_Special"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mylibB_PDB"
)
# TODO: The only .pdb available for a static library is that generated
# by the compiler /Fd option which is not the same as the linker /pdb.
# list(APPEND my_targets mylibB)

add_library(mylibC SHARED mylibC.c)
set_target_properties(mylibC PROPERTIES
    PDB_NAME "mylibC_Special"
)
list(APPEND my_targets mylibC)

add_library(mylibD STATIC mylibD.c)
set_target_properties(mylibD PROPERTIES
    PDB_NAME "mylibD_Special"
)
# TODO: See comment for mylibB.
# list(APPEND my_targets mylibD)

add_executable(myexe myexe.c)
set_target_properties(myexe PROPERTIES
    PDB_NAME "myexe_Special"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/myexe_PDB"
)
list(APPEND my_targets myexe)

target_link_libraries(myexe mylibA mylibB mylibC mylibD)

add_executable(myexe2 myexe2.c)
set_target_properties(myexe2 PROPERTIES
    PDB_NAME "myexe2_Special"
)
list(APPEND my_targets myexe2)

target_link_libraries(myexe2 mylibA mylibD)

#-----------------------------------------------------------------------------
# Check that PDB files actually appear where expected.

# The PDB_NAME and PDB_OUTPUT_DIRECTORY options do not work in VS 6.
if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 6")
  return()
endif()

set(pdbs "")
foreach(t ${my_targets})
  get_property(pdb_name TARGET ${t} PROPERTY PDB_NAME)
  get_property(pdb_dir TARGET ${t} PROPERTY PDB_OUTPUT_DIRECTORY)
  if(NOT pdb_dir)
    set(pdb_dir ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  list(APPEND pdbs ${pdb_dir}/${CMAKE_CFG_INTDIR}/${pdb_name}.pdb)
endforeach()
add_custom_target(check_pdbs ALL VERBATIM
  COMMAND ${CMAKE_COMMAND} -Dconfig=$<CONFIGURATION> "-Dpdbs=${pdbs}"
                           -P ${CMAKE_CURRENT_SOURCE_DIR}/check_pdbs.cmake
  )
add_dependencies(check_pdbs ${my_targets})
