project(XcodePrecompileHeaders CXX)

add_library(tgt foo.cpp)
target_precompile_headers(tgt PRIVATE stdafx.h)
