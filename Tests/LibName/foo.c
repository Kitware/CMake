#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void foo(void);
#ifdef _WIN32
__declspec(dllexport)
#endif
  void bar(void)
{
  foo();
}
