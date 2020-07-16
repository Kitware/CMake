

macro (add_ispc_test_macro name)
  add_test_macro("${name}" ${ARGN})
  set_property(TEST "${name}" APPEND
    PROPERTY LABELS "ISPC")
endmacro ()

# set (ISPC_IA_TARGETS "sse2-i32x4,sse4-i32x4,avx1-i32x8,avx2-i32x8,avx512knl-i32x16,avx512skx-i32x16")

add_ispc_test_macro(ISPC.Defines ISPCDefines)
add_ispc_test_macro(ISPC.ObjectLibrary ISPCObjectLibrary)
add_ispc_test_macro(ISPC.StaticLibrary ISPCStaticLibrary)
add_ispc_test_macro(ISPC.TryCompile ISPCTryCompile)
