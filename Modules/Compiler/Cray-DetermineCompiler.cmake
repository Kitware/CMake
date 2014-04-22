
set(_compiler_id_pp_test "defined(_CRAYC)")

set(_compiler_id_version_compute "
# define COMPILER_VERSION_MAJOR DEC(_RELEASE)
# define COMPILER_VERSION_MINOR DEC(_RELEASE_MINOR)")
