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

check_symbol_exists("arc4random_buf" "stdlib.h" HAVE_ARC4RANDOM_BUF)
if(NOT HAVE_ARC4RANDOM_BUF)
    check_symbol_exists("arc4random" "stdlib.h" HAVE_ARC4RANDOM)
endif()
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

if(HAVE_SYS_TYPES_H)
    check_c_source_compiles("
        #include <sys/types.h>
        int main(void) {
            const off_t offset = -123;
            (void)offset;
            return 0;
        }"
        HAVE_OFF_T)
endif()

if(NOT HAVE_OFF_T)
    set(off_t "long")
endif()

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

# If the compiler produces non-English messages and does not
# listen to CMake's request for English through environment variables
# LC_ALL/LC_MESSAGES/LANG, then command `check_c_compiler_flag` can produce
# false positives as seen with e.g. `cl` of MSVC 19.44.35217 configured
# to report errors in Italian language.
check_c_compiler_flag("-no-such-thing" _FLAG_DETECTION_UNUSABLE)

if (_FLAG_DETECTION_UNUSABLE)
    message(WARNING
        "Your compiler breaks CMake's command `check_c_compiler_flag`."
        " HINT: Is it configured to report errors in a language other"
        " than English?"
    )
    set(FLAG_WSTRICT_ALIASING FALSE)
    set(FLAG_VISIBILITY FALSE)
else()
    check_c_compiler_flag("-Wstrict-aliasing=3" FLAG_WSTRICT_ALIASING)
    check_c_compiler_flag("-fvisibility=hidden" FLAG_VISIBILITY)
endif()

check_library_exists(m cos "" _EXPAT_LIBM_FOUND)
