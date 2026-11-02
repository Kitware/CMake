// A shared or module library must be compiled with __DLL__.
#ifndef __DLL__
#  error "DLL compiled without __DLL__"
#endif
__declspec(dllexport) int dll_value()
{
  return static_cast<int>(std::string("dll").size());
}
