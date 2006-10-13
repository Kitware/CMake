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
# define KWSYS_PLATFORM_TEST_C_MAIN() \
  main()
# define KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv) \
  main(argc,argv) int argc; char* argv[];
#else
# define KWSYS_PLATFORM_TEST_C_MAIN() \
  main(void)
# define KWSYS_PLATFORM_TEST_C_MAIN_ARGS(argc, argv) \
  main(int argc, char* argv[])
#endif

/*--------------------------------------------------------------------------*/
#ifdef TEST_KWSYS_C_HAS_PTRDIFF_T
#include <stddef.h>
int f(ptrdiff_t n) { return n > 0; }
int KWSYS_PLATFORM_TEST_C_MAIN()
{
  char* p = 0;
  ptrdiff_t d = p - p;
  (void)d;
  return f(p - p);
}
#endif

/*--------------------------------------------------------------------------*/
#ifdef TEST_KWSYS_C_HAS_SSIZE_T
#include <unistd.h>
int f(ssize_t n) { return (int)n; }
int KWSYS_PLATFORM_TEST_C_MAIN()
{
  ssize_t n = 0;
  return f(n);
}
#endif
