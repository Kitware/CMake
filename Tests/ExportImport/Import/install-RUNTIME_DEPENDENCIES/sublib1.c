#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void dep6(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  void sublib1(void)
{
  dep6();
}
