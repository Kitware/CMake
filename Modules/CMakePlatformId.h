/* Identify known platforms by name.  */
static char const info_platform[] = "INFO:platform["
#if defined(__linux) || defined(__linux__) || defined(linux)
"Linux"
#elif defined(__CYGWIN__)
"Cygwin"
#elif defined(__MINGW32__)
"MinGW"
#elif defined(__APPLE__)
"Darwin"
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
"Windows"
#elif defined(__FreeBSD__) || defined(__FreeBSD)
"FreeBSD"
#elif defined(__NetBSD__) || defined(__NetBSD)
"NetBSD"
#elif defined(__OpenBSD__) || defined(__OPENBSD)
"OpenBSD"
#elif defined(__sun) || defined(sun)
"SunOS"
#elif defined(_AIX) || defined(__AIX) || defined(__AIX__) || defined(__aix) || defined(__aix__)
"AIX"
#elif defined(__sgi) || defined(__sgi__) || defined(_SGI)
"IRIX"
#elif defined(__hpux) || defined(__hpux__)
"HP-UX"
#elif defined(__BeOS) || defined(__BEOS__) || defined(_BEOS)
"BeOS"
#elif defined(__QNX__) || defined(__QNXNTO__)
"QNX"
#elif defined(__tru64) || defined(_tru64) || defined(__TRU64__)
"Tru64"
#elif defined(__riscos) || defined(__riscos__)
"RISCos"
#elif defined(__sinix) || defined(__sinix__) || defined(__SINIX__)
"SINIX"
#elif defined(__UNIX_SV__)
"UNIX_SV"
#elif defined(__bsdos__)
"BSDOS"
#elif defined(_MPRAS) || defined(MPRAS)
"MP-RAS"
#elif defined(__osf) || defined(__osf__)
"OSF1"
#elif defined(_SCO_SV) || defined(SCO_SV) || defined(sco_sv)
"SCO_SV"
#elif defined(__ultrix) || defined(__ultrix__) || defined(_ULTRIX)
"ULTRIX"
#elif defined(__XENIX__) || defined(_XENIX) || defined(XENIX)
"Xenix"
#else /* unknown platform */
""
#endif
"]";
