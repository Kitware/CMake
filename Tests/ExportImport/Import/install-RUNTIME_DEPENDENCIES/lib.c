#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void dep8(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  void lib(void)
{
  dep8();
}
