#if defined(_WIN32)
__declspec(dllimport)
#endif
  int foo(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  int bar(void)
{
  return foo();
}
