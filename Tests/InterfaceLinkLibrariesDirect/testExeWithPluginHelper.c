#ifdef _WIN32
__declspec(dllexport)
#endif
  int testExePluginAPI(int n)
{
  return n;
}

int main(void)
{
  return 0;
}
