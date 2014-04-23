
set(_compiler_id_pp_test "defined(__HP_aCC)")

set(_compiler_id_version_compute "
  /* __HP_aCC = VVRRPP */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__HP_aCC/10000)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__HP_aCC/100 % 100)
# define @PREFIX@COMPILER_VERSION_PATCH DEC(__HP_aCC     % 100)")
