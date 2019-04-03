add_library(NotObjLib INTERFACE)
add_library(A STATIC a.c $<TARGET_OBJECTS:NotObjLib>)
