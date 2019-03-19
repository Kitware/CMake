add_library( codegenexlib INTERFACE )
install(CODE "message( STATUS \"$<TARGET_PROPERTY:codegenexlib,NAME>\")")
