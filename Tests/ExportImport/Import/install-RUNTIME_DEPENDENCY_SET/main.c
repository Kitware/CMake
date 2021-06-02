#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void dep3(void);
#ifdef _WIN32
__declspec(dllimport)
#endif
  extern void dep4(void);

int main(void)
{
  dep3();
  dep4();
  return 0;
}
