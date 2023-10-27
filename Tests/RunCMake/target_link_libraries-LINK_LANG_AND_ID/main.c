
#if defined(_WIN32)
__declspec(dllimport)
#endif
  void func_c(void);

int main(void)
{
  func_c();

  return 0;
}
