enable_language(C)

add_library(foo1 STATIC empty.c)
target_include_directories(foo1 PUBLIC include)
target_link_libraries(foo1 PRIVATE foo2 foo3 foo4)

add_library(foo2 STATIC empty.c)
target_include_directories(foo2 PUBLIC $<TARGET_PROPERTY:foo1,INCLUDE_DIRECTORIES>)

add_library(foo3 STATIC empty.c)
target_include_directories(foo3 PUBLIC $<TARGET_PROPERTY:foo2,INCLUDE_DIRECTORIES>)

add_library(foo4 STATIC empty.c)
target_include_directories(foo4 PUBLIC $<TARGET_PROPERTY:foo3,INCLUDE_DIRECTORIES>)

add_library (foo5 SHARED empty.c)
set_property(TARGET foo5 PROPERTY INCLUDE_DIRECTORIES "$<$<COMPILE_LANGUAGE:CUDA>:/include/CUDA>" "$<$<COMPILE_LANGUAGE:Fortran>:/include/Fortran>")
set_property(TARGET foo5 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "$<$<COMPILE_LANGUAGE:CUDA>:/include/CUDA>" "$<$<COMPILE_LANGUAGE:Fortran>:/include/Fortran>")
set_property(TARGET foo5 PROPERTY CUSTOM ";;")

# Evaluate a genex that looks up INCLUDE_DIRECTORIES on multiple targets.
file(GENERATE OUTPUT out.txt CONTENT "INCLUDES1:$<TARGET_PROPERTY:foo4,INCLUDE_DIRECTORIES>\nINCLUDES2:>$<TARGET_PROPERTY:foo5,INTERFACE_INCLUDE_DIRECTORIES><\nINCLUDES3:>$<TARGET_PROPERTY:foo5,INCLUDE_DIRECTORIES><\nCUSTOM:>$<TARGET_PROPERTY:foo5,CUSTOM><\n")
