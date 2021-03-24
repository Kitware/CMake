#ifdef _WIN32
__declspec(dllimport)
#endif
  int ver_space(void);

int main(void)
{
  return ver_space();
}
