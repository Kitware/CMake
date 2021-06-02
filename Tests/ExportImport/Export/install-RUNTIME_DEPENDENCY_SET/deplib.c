#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void dep1(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  void deplib(void)
{
  dep1();
}
