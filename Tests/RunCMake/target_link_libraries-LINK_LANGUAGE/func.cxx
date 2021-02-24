
#if !defined(BUILD_STATIC) && defined(_WIN32)
#  define EXPORT_SYMBOL __declspec(dllexport)
#else
#  define EXPORT_SYMBOL
#endif

EXPORT_SYMBOL
void func_cxx()
{
}

extern "C" {
EXPORT_SYMBOL
void func_c_cxx()
{
}
}
