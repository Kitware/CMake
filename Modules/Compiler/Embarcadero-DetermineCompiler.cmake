
set(_compiler_id_pp_test "defined(__BORLANDC__) && defined(__CODEGEARC_VERSION__)")

set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR HEX(__CODEGEARC_VERSION__>>24 & 0x00FF)
# define @PREFIX@COMPILER_VERSION_MINOR HEX(__CODEGEARC_VERSION__>>16 & 0x00FF)
# define @PREFIX@COMPILER_VERSION_PATCH HEX(__CODEGEARC_VERSION__     & 0xFFFF)")
