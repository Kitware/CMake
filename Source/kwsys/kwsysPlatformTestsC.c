/* Macros to define main() in a cross-platform way.
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
