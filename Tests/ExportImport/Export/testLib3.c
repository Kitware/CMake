#if defined(_WIN32) || defined(__CYGWIN__)
# define testLib3_EXPORT __declspec(dllexport)
#else
# define testLib3_EXPORT
#endif

testLib3_EXPORT int testLib3(void) { return 0; }
