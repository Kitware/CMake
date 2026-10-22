/* The EMBT_RUNTIME_LIBRARY property and the target type select bcc64x driver
   flags.  Each flag defines characteristic preprocessor macros; assert them
   here so a wrong mapping fails the build.  The VERIFY_* switches are passed
   by the test for the selection under test. */

/* EMBT_RUNTIME_LIBRARY=Dynamic uses the dynamic RTL (-tR), which defines
   _RTLDLL. */
#ifdef VERIFY_RTLDLL
#  ifndef _RTLDLL
#    error "_RTLDLL not defined by the dynamic runtime selection"
#  endif
#else
#  ifdef _RTLDLL
#    error "_RTLDLL defined without the dynamic runtime selection"
#  endif
#endif

/* The dynamic RTL also defines USEPACKAGES (added by CMake). */
#ifdef VERIFY_USEPACKAGES
#  ifndef USEPACKAGES
#    error "USEPACKAGES not defined by the dynamic runtime selection"
#  endif
#else
#  ifdef USEPACKAGES
#    error "USEPACKAGES defined without the dynamic runtime selection"
#  endif
#endif

/* A shared library defines __DLL__. */
#ifdef VERIFY_DLL
#  ifndef __DLL__
#    error "__DLL__ not defined for a shared library"
#  endif
#else
#  ifdef __DLL__
#    error "__DLL__ defined for a non-shared target"
#  endif
#endif
