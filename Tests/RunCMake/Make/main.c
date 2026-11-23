int dep(void);
#if defined(_WIN32)
__declspec(dllexport)
#endif
int top(void)
{
  return dep();
}
