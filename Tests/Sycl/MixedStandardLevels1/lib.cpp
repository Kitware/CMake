#if defined(_MSVC_LANG)
#  if _MSVC_LANG < 202002L
#    error "CXX must use at least C++20"
#  endif
#elif __cplusplus < 202002L
#  error "CXX must use at least C++20"
#endif
int cxx_value()
{
  return 20;
}
