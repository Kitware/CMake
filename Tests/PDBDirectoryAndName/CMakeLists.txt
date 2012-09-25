cmake_minimum_required(VERSION 2.8)
project(PDBDirectoryAndName C)

if(NOT MSVC)
    MESSAGE(FATAL_ERROR "The PDB file test works only with MSVC")
endif()

add_library(mylibA SHARED mylibA.c)
set_target_properties(mylibA PROPERTIES
    PDB_NAME "mylibA_Special"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mylibA_PDB"
)

add_library(mylibB STATIC mylibB.c)
set_target_properties(mylibB PROPERTIES
    PDB_NAME "mylibB_Special"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mylibB_PDB"
)

add_library(mylibC SHARED mylibC.c)
set_target_properties(mylibC PROPERTIES
    PDB_NAME "mylibC_Special"
)

add_library(mylibD STATIC mylibD.c)
set_target_properties(mylibD PROPERTIES
    PDB_NAME "mylibD_Special"
)

add_executable(myexe myexe.c)
set_target_properties(myexe PROPERTIES
    PDB_NAME "myexe_Special"
    PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/myexe_PDB"
)

target_link_libraries(myexe mylibA mylibB mylibC mylibD)

add_executable(myexe2 myexe2.c)
set_target_properties(myexe2 PROPERTIES
    PDB_NAME "myexe2_Special"
)

target_link_libraries(myexe2 mylibA mylibD)
