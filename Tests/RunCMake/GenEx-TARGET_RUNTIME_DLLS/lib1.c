#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void lib2(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  void lib1(void)
{
  lib2();
}
