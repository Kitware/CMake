# Note that it is the policy settings at the end of the directory
# scope that will be used when deciding whether or not generator
# expressions should be evaluated in the installed code.
cmake_policy(VERSION 3.13)
cmake_policy(SET CMP0087 NEW)
add_library( codegenexlib INTERFACE )
install(CODE "message( STATUS \"$<TARGET_PROPERTY:codegenexlib,NAME>\")")
