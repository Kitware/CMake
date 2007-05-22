/* Identify known platforms by name.  */
#if defined(__linux) || defined(__linux__) || defined(linux)
#define _PLATFORM_ID "Linux"

#elif defined(__CYGWIN__)
#define _PLATFORM_ID "Cygwin"

#elif defined(__MINGW32__)
#define _PLATFORM_ID "MinGW"

#elif defined(__APPLE__)
#define _PLATFORM_ID "Darwin"

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define _PLATFORM_ID "Windows"

#elif defined(__FreeBSD__) || defined(__FreeBSD)
#define _PLATFORM_ID "FreeBSD"

#elif defined(__NetBSD__) || defined(__NetBSD)
#define _PLATFORM_ID "NetBSD"

#elif defined(__OpenBSD__) || defined(__OPENBSD)
#define _PLATFORM_ID "OpenBSD"

#elif defined(__sun) || defined(sun)
#define _PLATFORM_ID "SunOS"

#elif defined(_AIX) || defined(__AIX) || defined(__AIX__) || defined(__aix) || defined(__aix__)
#define _PLATFORM_ID "AIX"

#elif defined(__sgi) || defined(__sgi__) || defined(_SGI)
#define _PLATFORM_ID "IRIX"

#elif defined(__hpux) || defined(__hpux__)
#define _PLATFORM_ID "HP-UX"

#elif defined(__BeOS) || defined(__BEOS__) || defined(_BEOS)
#define _PLATFORM_ID "BeOS"

#elif defined(__QNX__) || defined(__QNXNTO__)
#define _PLATFORM_ID "QNX"

#elif defined(__tru64) || defined(_tru64) || defined(__TRU64__)
#define _PLATFORM_ID "Tru64"

#elif defined(__riscos) || defined(__riscos__)
#define _PLATFORM_ID "RISCos"

#elif defined(__sinix) || defined(__sinix__) || defined(__SINIX__)
#define _PLATFORM_ID "SINIX"

#elif defined(__UNIX_SV__)
#define _PLATFORM_ID "UNIX_SV"

#elif defined(__bsdos__)
#define _PLATFORM_ID "BSDOS"

#elif defined(_MPRAS) || defined(MPRAS)
#define _PLATFORM_ID "MP-RAS"

#elif defined(__osf) || defined(__osf__)
#define _PLATFORM_ID "OSF1"

#elif defined(_SCO_SV) || defined(SCO_SV) || defined(sco_sv)
#define _PLATFORM_ID "SCO_SV"

#elif defined(__ultrix) || defined(__ultrix__) || defined(_ULTRIX)
#define _PLATFORM_ID "ULTRIX"

#elif defined(__XENIX__) || defined(_XENIX) || defined(XENIX)
#define _PLATFORM_ID "Xenix"

#else /* unknown platform */
#define _PLATFORM_ID ""

#endif
static char const info_platform[] = "INFO:platform[" _PLATFORM_ID "]";
