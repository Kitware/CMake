
set(_compiler_id_pp_test "defined(__GNUC__)")

set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__GNUC__)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__GNUC_MINOR__)
# if defined(__GNUC_PATCHLEVEL__)
#  define @PREFIX@COMPILER_VERSION_PATCH DEC(__GNUC_PATCHLEVEL__)
# endif")
