
set(_compiler_id_pp_test "defined(__PGI)")

set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__PGIC__)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__PGIC_MINOR__)
# if defined(__PGIC_PATCHLEVEL__)
#  define @PREFIX@COMPILER_VERSION_PATCH DEC(__PGIC_PATCHLEVEL__)
# endif")
