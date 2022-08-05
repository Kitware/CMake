
#if !defined(STATIC_BASE)
#  if defined(_WIN32)
__declspec(dllimport)
#  endif
#endif
  void base();

#if defined(_WIN32)
__declspec(dllexport)
#endif
  void lib()
{
  base();
}
