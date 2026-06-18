enable_language(C)

# Each target's product PBXFileReference must get a deterministic object id so
# that a parent project embedding this one can keep a stable remoteGlobalIDString.
add_library(productStatic STATIC empty.c)
add_library(productShared SHARED empty.c)
add_executable(productExe empty.c)

# A target in a subdirectory exercises the relative-path component of the key.
add_subdirectory(DeterministicProductIdsSub)
