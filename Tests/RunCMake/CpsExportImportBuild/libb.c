extern
#ifdef _WIN32
__declspec(dllimport)
#endif
int answer(void);

#ifdef _WIN32
__declspec(dllexport)
#endif
int ask(void)
{
  return answer();
}
