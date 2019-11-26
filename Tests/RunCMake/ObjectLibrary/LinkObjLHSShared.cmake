project(LinkObjLHSShared C)

# Create a versioned shared library that does not build as part of "all".
add_library(OtherLib SHARED a.c)
target_compile_definitions(OtherLib INTERFACE REQUIRED PRIVATE COMPILE_FOR_SHARED_LIB)
set_target_properties(OtherLib PROPERTIES SOVERSION 0 VERSION 0.0.0 EXCLUDE_FROM_ALL ON)

add_library(AnObjLib OBJECT requires.c)
target_link_libraries(AnObjLib PUBLIC OtherLib)

add_executable(LinkObjLHSShared LinkObjLHSShared.c)
target_link_libraries(LinkObjLHSShared AnObjLib)

# Verify that our dependency on OtherLib generated its versioning symlinks.
add_custom_command(TARGET LinkObjLHSShared POST_BUILD COMMAND LinkObjLHSShared)
