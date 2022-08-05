
#if !defined(STATIC_BASE)
#  if defined(_WIN32)
__declspec(dllimport)
#  endif
#endif
  void base(void);

#if defined(_WIN32)
__declspec(dllexport)
#endif
  void lib(void)
{
  base();
}
