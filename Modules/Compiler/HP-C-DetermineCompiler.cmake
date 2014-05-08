
set(_compiler_id_pp_test "defined(__HP_cc)")

set(_compiler_id_version_compute "
  /* __HP_cc = VVRRPP */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__HP_cc/10000)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__HP_cc/100 % 100)
# define @PREFIX@COMPILER_VERSION_PATCH DEC(__HP_cc     % 100)")
