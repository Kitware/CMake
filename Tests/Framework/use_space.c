#ifdef _WIN32
__declspec(dllimport)
#endif
  int space(void);
int main(void)
{
  return space();
}
