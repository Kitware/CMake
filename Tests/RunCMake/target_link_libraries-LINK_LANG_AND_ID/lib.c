
#if defined(_WIN32)
__declspec(dllimport)
#endif
  void func_c();

void lib()
{
  func_c();
}
