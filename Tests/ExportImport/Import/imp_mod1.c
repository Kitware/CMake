#if defined(_WIN32) || defined(__CYGWIN__)
# define testExe2_IMPORT __declspec(dllimport)
#else
# define testExe2_IMPORT
#endif

testExe2_IMPORT int testExe2Func(void);

int imp_mod1()
{
  return testExe2Func();
}
