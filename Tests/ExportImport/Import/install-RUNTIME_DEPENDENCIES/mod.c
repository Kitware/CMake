#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void dep9(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  void mod(void)
{
  dep9();
}
