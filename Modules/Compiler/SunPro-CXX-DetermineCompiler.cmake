
set(_compiler_id_pp_test "defined(__SUNPRO_CC)")

set(_compiler_id_version_compute "
# if __SUNPRO_CC >= 0x5100
   /* __SUNPRO_CC = 0xVRRP */
#  define @PREFIX@COMPILER_VERSION_MAJOR HEX(__SUNPRO_CC>>12)
#  define @PREFIX@COMPILER_VERSION_MINOR HEX(__SUNPRO_CC>>4 & 0xFF)
#  define @PREFIX@COMPILER_VERSION_PATCH HEX(__SUNPRO_CC    & 0xF)
# else
   /* __SUNPRO_CC = 0xVRP */
#  define @PREFIX@COMPILER_VERSION_MAJOR HEX(__SUNPRO_CC>>8)
#  define @PREFIX@COMPILER_VERSION_MINOR HEX(__SUNPRO_CC>>4 & 0xF)
#  define @PREFIX@COMPILER_VERSION_PATCH HEX(__SUNPRO_CC    & 0xF)
# endif")
