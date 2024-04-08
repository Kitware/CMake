
template <long l>
struct Outputter;

#define CXX_STD_98 199711L
#define CXX_STD_11 201103L
#define CXX_STD_14 201402L
#define CXX_STD_17 201703L
#define CXX_STD_20 202002L
#define CXX_STD_23 202302L

#if defined(__INTEL_COMPILER) && defined(_MSVC_LANG)
#  if _MSVC_LANG > CXX_STD_17
#    define CXX_STD _MSVC_LANG
#  elif _MSVC_LANG == CXX_STD_17 && defined(__cpp_aggregate_paren_init)
#    define CXX_STD CXX_STD_20
#  elif _MSVC_LANG > CXX_STD_14 && __cplusplus > CXX_STD_17
#    define CXX_STD CXX_STD_20
#  elif _MSVC_LANG > CXX_STD_14
#    define CXX_STD CXX_STD_17
#  elif defined(__INTEL_CXX11_MODE__) && defined(__cpp_aggregate_nsdmi)
#    define CXX_STD CXX_STD_14
#  elif defined(__INTEL_CXX11_MODE__)
#    define CXX_STD CXX_STD_11
#  else
#    define CXX_STD CXX_STD_98
#  endif
#elif defined(_MSC_VER) && defined(_MSVC_LANG)
#  define CXX_STD _MSVC_LANG
#elif defined(__NVCOMPILER)
#  if __cplusplus == CXX_STD_17 && defined(__cpp_aggregate_paren_init)
#    define CXX_STD CXX_STD_20
#  else
#    define CXX_STD __cplusplus
#  endif
#elif defined(__INTEL_COMPILER) || defined(__PGI)
#  if __cplusplus == CXX_STD_11 && defined(__cpp_namespace_attributes)
#    define CXX_STD CXX_STD_17
#  elif __cplusplus == CXX_STD_11 && defined(__cpp_aggregate_nsdmi)
#    define CXX_STD CXX_STD_14
#  else
#    define CXX_STD __cplusplus
#  endif
#elif (defined(__IBMCPP__) || defined(__ibmxl__)) && defined(__linux__)
#  if __cplusplus == CXX_STD_11 && defined(__cpp_aggregate_nsdmi)
#    define CXX_STD CXX_STD_14
#  else
#    define CXX_STD __cplusplus
#  endif
#elif __cplusplus == 1 && defined(__GXX_EXPERIMENTAL_CXX0X__)
#  define CXX_STD CXX_STD_11
#else
#  define CXX_STD __cplusplus
#endif

#if DEFAULT_CXX23
#  if CXX_STD <= CXX_STD_20
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX20
#  if CXX_STD <= CXX_STD_17
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX17
#  if CXX_STD <= CXX_STD_14
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX14
#  if CXX_STD <= CXX_STD_11
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX11
#  if CXX_STD != CXX_STD_11
Outputter<CXX_STD> o;
#  endif
#else
#  if !DEFAULT_CXX98
#    error Buildsystem error
#  endif
#  if CXX_STD != CXX_STD_98 && CXX_STD != 1 &&                                \
    !defined(__GXX_EXPERIMENTAL_CXX0X__)
Outputter<CXX_STD> o;
#  endif
#endif

int main()
{
  return 0;
}
