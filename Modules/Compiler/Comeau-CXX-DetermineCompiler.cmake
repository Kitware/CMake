
set(_compiler_id_pp_test "defined(__COMO__)")

set(_compiler_id_version_compute "
  /* __COMO_VERSION__ = VRR */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__COMO_VERSION__ / 100)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__COMO_VERSION__ % 100)")
