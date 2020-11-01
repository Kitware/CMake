project(VsPrecompileHeadersReuseFromCompilePDBName CXX)

add_library(a SHARED empty.cxx)
target_precompile_headers(a PRIVATE <windows.h>)

add_library(b SHARED empty.cxx)
target_precompile_headers(b REUSE_FROM a)

set_target_properties(b PROPERTIES COMPILE_PDB_NAME b)
