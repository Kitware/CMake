
/* Size of a pointer-to-data in bytes.  */
#define SIZEOF_DPTR (sizeof(void*))
char const info_sizeof_dptr[] = {
  /* clang-format off */
  'I', 'N', 'F', 'O', ':', 's', 'i', 'z', 'e', 'o', 'f', '_', 'd', 'p', 't',
  'r', '[', ('0' + ((SIZEOF_DPTR / 10) % 10)), ('0' + (SIZEOF_DPTR % 10)), ']',
  '\0'
  /* clang-format on */
};

/* Byte order.  Only one of these will have bytes in the right order.  */
static unsigned short const info_byte_order_big_endian[] = {
  /* INFO:byte_order string for BIG_ENDIAN */
  0x494E, 0x464F, 0x3A62, 0x7974, 0x655F, 0x6F72, 0x6465, 0x725B,
  0x4249, 0x475F, 0x454E, 0x4449, 0x414E, 0x5D00, 0x0000
};
static unsigned short const info_byte_order_little_endian[] = {
  /* INFO:byte_order string for LITTLE_ENDIAN */
  0x4E49, 0x4F46, 0x623A, 0x7479, 0x5F65, 0x726F, 0x6564, 0x5B72,
  0x494C, 0x5454, 0x454C, 0x455F, 0x444E, 0x4149, 0x5D4E, 0x0000
};

/* Application Binary Interface.  */

/* Check for (some) ARM ABIs.
 * See e.g. http://wiki.debian.org/ArmEabiPort for some information on this. */
#if defined(__GNU__) && defined(__ELF__) && defined(__ARM_EABI__)
#  define ABI_ID "ELF ARMEABI"
#elif defined(__GNU__) && defined(__ELF__) && defined(__ARMEB__)
#  define ABI_ID "ELF ARM"
#elif defined(__GNU__) && defined(__ELF__) && defined(__ARMEL__)
#  define ABI_ID "ELF ARM"

#elif defined(__linux__) && defined(__ELF__) && defined(__amd64__) &&         \
  defined(__ILP32__)
#  define ABI_ID "ELF X32"

#elif defined(__ELF__)
#  define ABI_ID "ELF"
#endif

#if defined(ABI_ID)
static char const info_abi[] = "INFO:abi[" ABI_ID "]";
#endif
