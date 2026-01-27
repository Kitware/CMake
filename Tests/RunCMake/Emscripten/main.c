
#if defined(_WIN32)
__declspec(dllimport)
#endif
  void lib(void);

#if defined(_WIN32)
__declspec(dllimport)
#endif
  void unref(void);

int main(void)
{
  lib();
  unref();

  return 0;
}
