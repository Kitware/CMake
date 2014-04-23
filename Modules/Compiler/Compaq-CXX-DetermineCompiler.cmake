
set(_compiler_id_pp_test "defined(__DECCXX)")

set(_compiler_id_version_compute "
  /* __DECCXX_VER = VVRRTPPPP */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__DECCXX_VER/10000000)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__DECCXX_VER/100000  % 100)
# define @PREFIX@COMPILER_VERSION_PATCH DEC(__DECCXX_VER         % 10000)")
