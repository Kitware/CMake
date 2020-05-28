
cmake_minimum_required(VERSION 3.17)

add_library(lib-global SHARED IMPORTED GLOBAL)
add_library(alias-lib-global ALIAS lib-global)

add_library(lib-local SHARED IMPORTED)
add_library(alias-lib-local ALIAS lib-local)

add_library(lib SHARED IMPORTED)
add_library(alias-lib ALIAS lib)
# switch from local to global
set_property (TARGET lib PROPERTY IMPORTED_GLOBAL TRUE)


file(GENERATE OUTPUT alias_global.txt CONTENT "$<TARGET_PROPERTY:lib-global,IMPORTED_GLOBAL>($<TARGET_PROPERTY:alias-lib-global,ALIASED_TARGET>):$<TARGET_PROPERTY:alias-lib-global,ALIAS_GLOBAL>\n$<TARGET_PROPERTY:lib-local,IMPORTED_GLOBAL>($<TARGET_PROPERTY:alias-lib-local,ALIASED_TARGET>):$<TARGET_PROPERTY:alias-lib-local,ALIAS_GLOBAL>\n$<TARGET_PROPERTY:lib,IMPORTED_GLOBAL>($<TARGET_PROPERTY:alias-lib,ALIASED_TARGET>):$<TARGET_PROPERTY:alias-lib,ALIAS_GLOBAL>\n")
