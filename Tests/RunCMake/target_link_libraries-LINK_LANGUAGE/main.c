
#if defined(C_USE_CXX)
void func_c_cxx(void);
#else
#  if defined(_WIN32)
__declspec(dllimport)
#  endif
  void func_c(void);
#endif

int main(void)
{
#if defined(C_USE_CXX)
  func_c_cxx();
#else
  func_c();
#endif

  return 0;
}
