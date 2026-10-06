set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(C)

set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "ProgramDatabase")

add_library(empty STATIC empty.c)
target_precompile_headers(empty PUBLIC
  <stdio.h>
  <string.h>
)
target_include_directories(empty PUBLIC include)

# Targets reusing the same PCH and sharing a compiler PDB directory,
# both in the same directory and in different directories.
function(add_reusing_library name)
  add_library(${name} OBJECT ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/foo.c)
  target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/include)
  set_property(TARGET ${name} PROPERTY
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdb")
  target_precompile_headers(${name} REUSE_FROM empty)
endfunction()

add_reusing_library(foo1)
add_reusing_library(foo2)
add_subdirectory(shared_pdb_subdir)
