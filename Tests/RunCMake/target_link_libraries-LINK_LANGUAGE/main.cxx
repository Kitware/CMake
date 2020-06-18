
#if defined(_WIN32)
__declspec(dllimport)
#endif
  void func_cxx();

int main()
{
  func_cxx();

  return 0;
}
