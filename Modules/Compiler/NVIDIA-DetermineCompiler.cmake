
set(_compiler_id_pp_test "defined(__NVCC__)")

set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@(__CUDACC_VER_MAJOR__)
# define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(__CUDACC_VER_MINOR__)
# define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@(__CUDACC_VER_BUILD__)")
