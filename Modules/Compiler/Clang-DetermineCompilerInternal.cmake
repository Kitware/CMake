
set(_compiler_id_version_compute "
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(__clang_major__)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(__clang_minor__)
# define @PREFIX@COMPILER_VERSION_PATCH DEC(__clang_patchlevel__)
# if defined(_MSC_VER)
#  define @PREFIX@SIMULATE_ID \"MSVC\"
   /* _MSC_VER = VVRR */
#  define @PREFIX@SIMULATE_VERSION_MAJOR DEC(_MSC_VER / 100)
#  define @PREFIX@SIMULATE_VERSION_MINOR DEC(_MSC_VER % 100)
# endif")
