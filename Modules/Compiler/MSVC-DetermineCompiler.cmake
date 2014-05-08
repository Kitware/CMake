
set(_compiler_id_pp_test "defined(_MSC_VER)")

set(_compiler_id_version_compute "
  /* _MSC_VER = VVRR */
# define @PREFIX@COMPILER_VERSION_MAJOR DEC(_MSC_VER / 100)
# define @PREFIX@COMPILER_VERSION_MINOR DEC(_MSC_VER % 100)
# if defined(_MSC_FULL_VER)
#  if _MSC_VER >= 1400
    /* _MSC_FULL_VER = VVRRPPPPP */
#   define @PREFIX@COMPILER_VERSION_PATCH DEC(_MSC_FULL_VER % 100000)
#  else
    /* _MSC_FULL_VER = VVRRPPPP */
#   define @PREFIX@COMPILER_VERSION_PATCH DEC(_MSC_FULL_VER % 10000)
#  endif
# endif
# if defined(_MSC_BUILD)
#  define @PREFIX@COMPILER_VERSION_TWEAK DEC(_MSC_BUILD)
# endif")
