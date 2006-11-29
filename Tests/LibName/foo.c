#ifdef _WIN32
__declspec(dllimport) 
#endif
  void foo();
#ifdef _WIN32
__declspec(dllexport)  
#endif
  void bar()
{
  foo();
}
