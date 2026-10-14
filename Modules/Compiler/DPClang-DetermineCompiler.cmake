
set(_compiler_id_pp_test "defined(__clang__) && defined(__DPCPP__) && defined(__dpcpp_major__) && __dpcpp_major__ >= 7 && defined(__dpcpp_minor__) && defined(__dpcpp_patchlevel__) && !defined(__INTEL_CLANG_COMPILER) && !defined(__INTEL_LLVM_COMPILER)")

set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@(__dpcpp_major__)
# define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(__dpcpp_minor__)
# define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@(__dpcpp_patchlevel__)
# define @PREFIX@COMPILER_VERSION_INTERNAL_STR __clang_version__
#if defined(_MSC_VER)
# define @PREFIX@SIMULATE_VERSION_MAJOR @MACRO_DEC@(_MSC_VER / 100)
# define @PREFIX@SIMULATE_VERSION_MINOR @MACRO_DEC@(_MSC_VER % 100)
#endif")

set(_compiler_id_simulate "
#if defined(_MSC_VER)
# define @PREFIX@SIMULATE_ID \"MSVC\"
#endif")
