set(CMAKE_INTERMEDIATE_DIR_STRATEGY SHORT CACHE STRING "" FORCE)

enable_language(C)
add_library(UnityBuildPCH STATIC UnityBuildPCH.c)
target_precompile_headers(UnityBuildPCH PRIVATE UnityBuildPCH.h)
set_property(TARGET UnityBuildPCH PROPERTY UNITY_BUILD ON)
