#ifdef _WIN32
__declspec(dllimport) 
#endif
  void bar();

int main(int ac, char** av)
{
  bar();
  return 0;
}
