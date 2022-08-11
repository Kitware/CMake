#ifdef _WIN32
__declspec(dllexport)
#endif
  int testSharedLibWithHelper(int n)
{
  return n;
}
