
#if defined(_WIN32)
__declspec(dllimport)
#endif
  void lib();

#if defined(_WIN32)
__declspec(dllimport)
#endif
  void unref();

int main()
{
  lib();
  unref();

  return 0;
}
