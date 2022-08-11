extern int testExePluginHelperObj(int n);

#ifdef testExePluginHelperObj_NO_OBJECT
int testExePluginHelperObj(int n)
{
  return n;
}
#endif

#if defined(_WIN32)
__declspec(dllimport)
#endif
  int testExePluginAPI(int n);

#if defined(_WIN32)
__declspec(dllexport)
#endif
  int testExePlugin(int n)
{
  return testExePluginAPI(n) + testExePluginHelperObj(n);
}
