int middle(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
  int top(void)
{
  return middle() + 2;
}
