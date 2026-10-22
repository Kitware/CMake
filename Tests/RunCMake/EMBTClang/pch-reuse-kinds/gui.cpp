// A GUI (WIN32) executable must be compiled without __CONSOLE__ and __DLL__.
#if defined(__CONSOLE__) || defined(__DLL__)
#  error "GUI target compiled with __CONSOLE__ or __DLL__"
#endif
extern "C" int __stdcall WinMain(void*, void*, char*, int)
{
  return static_cast<int>(std::string().size());
}
