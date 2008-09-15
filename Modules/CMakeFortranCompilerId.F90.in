PROGRAM CMakeFortranCompilerId
!     Identify the compiler
#if defined(__INTEL_COMPILER) || defined(__ICC)
   PRINT *, 'INFO:compiler[Intel]'
#elif defined(__SUNPRO_F90) || defined(__SUNPRO_F95)
   PRINT *, 'INFO:compiler[SunPro]'
#elif defined(__GNUC__)
   PRINT *, 'INFO:compiler[GNU]'
#elif defined(__IBM__) || defined(__IBMC__)
   PRINT *, 'INFO:compiler[VisualAge]'
#elif defined(__PGI)
   PRINT *, 'INFO:compiler[PGI]'
#elif defined(_COMPILER_VERSION)
   PRINT *, 'INFO:compiler[MIPSpro]'
!     This compiler is either not known or is too old to define an
!     identification macro.  Try to identify the platform and guess that
!     it is the native compiler.
#elif defined(_AIX) || defined(__AIX) || defined(__AIX__) || defined(__aix) || defined(__aix__)
   PRINT *, 'INFO:compiler[VisualAge]'
#elif defined(__sgi) || defined(__sgi__) || defined(_SGI)
   PRINT *, 'INFO:compiler[MIPSpro]'
#elif defined(__hpux) || defined(__hpux__)
   PRINT *, 'INFO:compiler[HP]'
#else
   PRINT *, 'INFO:compiler[]'
#endif

!     Identify the platform
#if defined(__linux) || defined(__linux__) || defined(linux)
   PRINT *, 'INFO:platform[Linux]'
#elif defined(__CYGWIN__)
   PRINT *, 'INFO:platform[Cygwin]'
#elif defined(__MINGW32__)
   PRINT *, 'INFO:platform[MinGW]'
#elif defined(__APPLE__)
   PRINT *, 'INFO:platform[Darwin]'
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
   PRINT *, 'INFO:platform[Windows]'
#elif defined(__FreeBSD__) || defined(__FreeBSD)
   PRINT *, 'INFO:platform[FreeBSD]'
#elif defined(__NetBSD__) || defined(__NetBSD)
   PRINT *, 'INFO:platform[NetBSD]'
#elif defined(__OpenBSD__) || defined(__OPENBSD)
   PRINT *, 'INFO:platform[OpenBSD]'
#elif defined(__sun) || defined(sun)
   PRINT *, 'INFO:platform[SunOS]'
#elif defined(_AIX) || defined(__AIX) || defined(__AIX__) || defined(__aix) || defined(__aix__)
   PRINT *, 'INFO:platform[AIX]'
#elif defined(__sgi) || defined(__sgi__) || defined(_SGI)
   PRINT *, 'INFO:platform[IRIX]'
#elif defined(__hpux) || defined(__hpux__)
   PRINT *, 'INFO:platform[HP-UX]'
#elif defined(__HAIKU) || defined(__HAIKU__) || defined(_HAIKU)
   PRINT *, 'INFO:platform[Haiku]'
! Haiku also defines __BEOS__ so we must 
! put it prior to the check for __BEOS__
#elif defined(__BeOS) || defined(__BEOS__) || defined(_BEOS)
   PRINT *, 'INFO:platform[BeOS]'
#elif defined(__QNX__) || defined(__QNXNTO__)
   PRINT *, 'INFO:platform[QNX]'
#elif defined(__tru64) || defined(_tru64) || defined(__TRU64__)
   PRINT *, 'INFO:platform[Tru64]'
#elif defined(__riscos) || defined(__riscos__)
   PRINT *, 'INFO:platform[RISCos]'
#elif defined(__sinix) || defined(__sinix__) || defined(__SINIX__)
   PRINT *, 'INFO:platform[SINIX]'
#elif defined(__UNIX_SV__)
   PRINT *, 'INFO:platform[UNIX_SV]'
#elif defined(__bsdos__)
   PRINT *, 'INFO:platform[BSDOS]'
#elif defined(_MPRAS) || defined(MPRAS)
   PRINT *, 'INFO:platform[MP-RAS]'
#elif defined(__osf) || defined(__osf__)
   PRINT *, 'INFO:platform[OSF1]'
#elif defined(_SCO_SV) || defined(SCO_SV) || defined(sco_sv)
   PRINT *, 'INFO:platform[SCO_SV]'
#elif defined(__ultrix) || defined(__ultrix__) || defined(_ULTRIX)
   PRINT *, 'INFO:platform[ULTRIX]'
#elif defined(__XENIX__) || defined(_XENIX) || defined(XENIX)
   PRINT *, 'INFO:platform[Xenix]'
#else
   PRINT *, 'INFO:platform[]'
#endif
END PROGRAM
