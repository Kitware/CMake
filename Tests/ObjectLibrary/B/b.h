#ifdef A_DEF
# error "A_DEF must not be defined"
#endif
#ifndef B_DEF
# error "B_DEF not defined"
#endif
#if defined(_WIN32) && defined(Bexport)
# define EXPORT_B __declspec(dllexport)
#else
# define EXPORT_B
#endif
