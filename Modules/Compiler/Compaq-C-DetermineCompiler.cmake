
set(_compiler_id_pp_test "defined(__DECC)")

set(_compiler_id_version_compute "
  /* __DECC_VER = VVRRTPPPP */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__DECC_VER/10000000)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__DECC_VER/100000  % 100)
# define @PREFIX@COMPILER_VERSION_PATCH DEC(__DECC_VER         % 10000)")
