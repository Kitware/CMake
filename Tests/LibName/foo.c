__declspec(dllimport) void foo();
__declspec(dllexport)  void bar()
{
  foo();
}
