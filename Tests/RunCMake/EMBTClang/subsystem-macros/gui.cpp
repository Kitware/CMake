// A GUI (WIN32) target must NOT be compiled with __CONSOLE__.
#ifdef __CONSOLE__
#  error "GUI target compiled with __CONSOLE__"
#endif
extern "C" int __stdcall WinMain(void*, void*, char*, int)
{
  return 0;
}
