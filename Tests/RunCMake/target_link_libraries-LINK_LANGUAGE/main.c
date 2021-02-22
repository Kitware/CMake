
#if defined(C_USE_CXX)
void func_c_cxx();
#else
#  if defined(_WIN32)
__declspec(dllimport)
#  endif
  void func_c();
#endif

int main()
{
#if defined(C_USE_CXX)
  func_c_cxx();
#else
  func_c();
#endif

  return 0;
}
