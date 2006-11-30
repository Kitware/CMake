#ifdef _WIN32
__declspec(dllimport) 
#endif
extern  void bar();

int main(int ac, char** av)
{
  bar();
  return 0;
}
