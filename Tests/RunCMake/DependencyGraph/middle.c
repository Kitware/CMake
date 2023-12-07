#ifdef _WIN32
__declspec(dllimport)
#endif
  int bottom(void);

int middle(void)
{
  return bottom() + 19;
}
