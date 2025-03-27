#include <iostream>

#include "check_predefs.h"

#define TO_STRING(x) TO_STRING0(x)
#define TO_STRING0(x) #x

int main()
{
  int ret = 0;
#if defined(__STRICT_ANSI__)
#  if !defined(CHECK___STRICT_ANSI__)
  std::cout << "__STRICT_ANSI__: Expected " << TO_STRING(__STRICT_ANSI__)
            << " but it is not defined.\n";
  ret = 1;
#  elif __STRICT_ANSI__ != CHECK___STRICT_ANSI__
  std::cout << "__STRICT_ANSI__: Expected " << TO_STRING(__STRICT_ANSI__)
            << " but got: " << TO_STRING(CHECK___STRICT_ANSI__) << "\n";
  ret = 1;
#  endif
#elif defined(CHECK___STRICT_ANSI__)
  std::cout << "__STRICT_ANSI__: Expected undefined but got: "
            << TO_STRING(CHECK___STRICT_ANSI__) << "\n";
  ret = 1;
#endif

#if defined(__cplusplus)
#  if !defined(CHECK___cplusplus)
  std::cout << "__cplusplus: Expected " << TO_STRING(__cplusplus)
            << " but it is not defined.\n";
  ret = 1;
#  elif __cplusplus != CHECK___cplusplus
  std::cout << "__cplusplus: Expected " << TO_STRING(__cplusplus)
            << " but got: " << TO_STRING(CHECK___cplusplus) << "\n";
  ret = 1;
#  endif
#elif defined(CHECK___cplusplus)
  std::cout << "__cplusplus: Expected undefined but got: "
            << TO_STRING(CHECK___cplusplus) << "\n";
  ret = 1;
#endif

#if defined(_MSVC_LANG)
#  if !defined(CHECK__MSVC_LANG)
  std::cout << "_MSVC_LANG: Expected " << TO_STRING(_MSVC_LANG)
            << " but it is not defined.\n";
  ret = 1;
#  elif _MSVC_LANG != CHECK__MSVC_LANG
  std::cout << "_MSVC_LANG: Expected " << TO_STRING(_MSVC_LANG)
            << " but got: " << TO_STRING(CHECK__MSVC_LANG) << "\n";
  ret = 1;
#  endif
#elif defined(CHECK__MSVC_LANG)
  std::cout << "_MSVC_LANG: Expected undefined but got: "
            << TO_STRING(CHECK__MSVC_LANG) << "\n";
  ret = 1;
#endif

  return ret;
}
