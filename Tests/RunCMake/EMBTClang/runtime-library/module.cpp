#include <string>

extern "C" __declspec(dllexport) int rtlmod_size()
{
  std::string s = "x";
  return static_cast<int>(s.size());
}

#ifdef __DLL__
extern "C" __declspec(dllexport) int rtlmod_has_dll_macro()
{
  return 1;
}
#endif
