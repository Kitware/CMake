

macro (add_ispc_test_macro name)
  add_test_macro("${name}" ${ARGN})
  set_property(TEST "${name}" APPEND
    PROPERTY LABELS "ISPC")
endmacro ()

add_ispc_test_macro(ISPC.ChainedStaticLibraries ISPCChainedStaticLibraries)
add_ispc_test_macro(ISPC.CustomHeaderSuffix ISPCCustomHeaderSuffix)
add_ispc_test_macro(ISPC.Defines ISPCDefines)
add_ispc_test_macro(ISPC.DynamicLibrary ISPCDynamicLibrary)
add_ispc_test_macro(ISPC.ObjectGenex ISPCObjectGenex)
add_ispc_test_macro(ISPC.ObjectLibrary ISPCObjectLibrary)
add_ispc_test_macro(ISPC.ResponseAndDefine ISPCResponseAndDefine)
add_ispc_test_macro(ISPC.StaticLibrary ISPCStaticLibrary)
add_ispc_test_macro(ISPC.SystemIncludes ISPCSystemIncludes)
add_ispc_test_macro(ISPC.TryCompile ISPCTryCompile)
