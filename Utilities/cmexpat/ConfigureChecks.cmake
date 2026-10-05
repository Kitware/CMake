include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(TestBigEndian)

check_include_file("dlfcn.h" HAVE_DLFCN_H)
check_include_file("fcntl.h" HAVE_FCNTL_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
check_include_file("memory.h" HAVE_MEMORY_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("stdlib.h" HAVE_STDLIB_H)
check_include_file("strings.h" HAVE_STRINGS_H)
check_include_file("string.h" HAVE_STRING_H)
check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("unistd.h" HAVE_UNISTD_H)

check_symbol_exists("getpagesize" "unistd.h" HAVE_GETPAGESIZE)
check_symbol_exists("mmap" "sys/mman.h" HAVE_MMAP)
check_symbol_exists("getrandom" "sys/random.h" HAVE_GETRANDOM)

check_c_source_compiles("
    #if ! defined(_DEFAULT_SOURCE)
    # define _DEFAULT_SOURCE 1 /* for glibc */
    #endif
    #include <stdlib.h>
    int main(void) {
      char dummy[123];
      arc4random_buf(dummy, 0U);
      return 0;
    }"
    HAVE_ARC4RANDOM_BUF)

check_c_source_compiles("
    #if ! defined(_DEFAULT_SOURCE)
    # define _DEFAULT_SOURCE 1 /* for glibc */
    #endif
    #include <stdlib.h>
    int main(void) {
        arc4random();
        return 0;
    }"
    HAVE_ARC4RANDOM)

set(CMAKE_REQUIRED_LIBRARIES)

#/* Define to 1 if you have the ANSI C header files. */
check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

test_big_endian(WORDS_BIGENDIAN)
#/* 1234 = LIL_ENDIAN, 4321 = BIGENDIAN */
if(WORDS_BIGENDIAN)
    set(BYTEORDER 4321)
else(WORDS_BIGENDIAN)
    set(BYTEORDER 1234)
endif(WORDS_BIGENDIAN)

check_c_source_compiles("
        // NOTE: Please keep this block in sync with its two siblings in files
        //       `configure.ac` and `lib/random_getentropy.c`!
        #if defined(__APPLE__)
        #  include <sys/random.h>
        #else
        #  if defined(__GLIBC__) && ! defined(_DEFAULT_SOURCE)
        #    define _DEFAULT_SOURCE 1
        #  endif
        #  if ! defined(_GNU_SOURCE)
        #    define _GNU_SOURCE 1 /* for musl */
        #  endif
        #  include <unistd.h>
        #endif // ! defined(__APPLE__)

        int main(void) {
            return getentropy(NULL, 0U);
        }
    "
    HAVE_GETENTROPY)

check_c_source_compiles("
        #define _GNU_SOURCE
        #include <stdlib.h>  /* for NULL */
        #include <unistd.h>  /* for syscall */
        #include <sys/syscall.h>  /* for SYS_getrandom */
        int main(void) {
            syscall(SYS_getrandom, NULL, 0, 0);
            return 0;
        }"
    HAVE_SYSCALL_GETRANDOM)
