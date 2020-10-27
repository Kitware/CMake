
set(_compiler_id_pp_test "defined(__clang__) && defined(__INTEL_CLANG_COMPILER)")

include("${CMAKE_CURRENT_LIST_DIR}/Clang-DetermineCompilerInternal.cmake")

string(APPEND _compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_TWEAK @MACRO_DEC@(__INTEL_CLANG_COMPILER)")
