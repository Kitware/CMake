#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void dep7(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  void sublib2(void)
{
  dep7();
}
