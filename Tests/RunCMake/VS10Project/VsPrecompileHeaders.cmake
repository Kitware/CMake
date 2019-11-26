project(VsPrecompileHeaders CXX)

add_library(tgt SHARED empty.cxx)
target_precompile_headers(tgt PRIVATE stdafx.h)
