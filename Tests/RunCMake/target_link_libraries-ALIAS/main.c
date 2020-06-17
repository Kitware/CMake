
#if defined(_WIN32)
__declspec(dllimport)
#endif
  void func_c();

int main()
{
  func_c();

  return 0;
}
