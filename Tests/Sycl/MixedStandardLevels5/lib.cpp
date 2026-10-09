#if defined(_MSVC_LANG)
#  if _MSVC_LANG != 201402L
#    error "CXX_STANDARD was not honored"
#  endif
#elif __cplusplus != 201402L
#  error "CXX_STANDARD was not honored"
#endif
int cxx_value()
{
  return 14;
}
