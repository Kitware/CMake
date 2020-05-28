/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing#kwsys for details.  */
/*
  Macros to define main() in a cross-platform way.

  Usage:

    int KWSYS_PLATFORM_TEST_C_MAIN()
    {
      return 0;
    }

    int KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv)
    {
      (void)argc; (void)argv;
      return 0;
    }
*/
#if defined(__CLASSIC_C__)
#  define KWSYS_PLATFORM_TEST_C_MAIN() main()
#  define KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv)                         \
    main(argc, argv) int argc;                                                \
    char* argv[];
#else
#  define KWSYS_PLATFORM_TEST_C_MAIN() main(void)
#  define KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv)                         \
    main(int argc, char* argv[])
#endif

#ifdef TEST_KWSYS_C_HAS_PTRDIFF_T
#  include <stddef.h>
int f(ptrdiff_t n)
{
  return n > 0;
}
int KWSYS_PLATFORM_TEST_C_MAIN()
{
  char* p = 0;
  ptrdiff_t d = p - p;
  (void)d;
  return f(p - p);
}
#endif

#ifdef TEST_KWSYS_C_HAS_SSIZE_T
#  include <unistd.h>
int f(ssize_t n)
{
  return (int)n;
}
int KWSYS_PLATFORM_TEST_C_MAIN()
{
  ssize_t n = 0;
  return f(n);
}
#endif

#ifdef TEST_KWSYS_C_HAS_CLOCK_GETTIME_MONOTONIC
#  if defined(__APPLE__)
#    include <AvailabilityMacros.h>
#    if MAC_OS_X_VERSION_MIN_REQUIRED < 101200
#      error "clock_gettime not available on macOS < 10.12"
#    endif
#  endif
#  include <time.h>
int KWSYS_PLATFORM_TEST_C_MAIN()
{
  struct timespec ts;
  return clock_gettime(CLOCK_MONOTONIC, &ts);
}
#endif
