enable_language(ISPC)
add_library(objs OBJECT obj1.ispc obj2.ispc)
set_target_properties(objs PROPERTIES ISPC_INSTRUCTION_SETS "sse2-i32x4;sse4-i16x8;avx1-i32x16;avx2-i32x4")
install(FILES $<TARGET_OBJECTS:objs> DESTINATION objs)
