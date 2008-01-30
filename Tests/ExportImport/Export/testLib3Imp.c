#if defined(_WIN32) || defined(__CYGWIN__)
# define testLib3Imp_EXPORT __declspec(dllexport)
#else
# define testLib3Imp_EXPORT
#endif

testLib3Imp_EXPORT int testLib3Imp(void) { return 0; }
