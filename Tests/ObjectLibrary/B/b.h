#ifdef A
# error "A must not be defined"
#endif
#ifndef B
# error "B not defined"
#endif
#if defined(_WIN32) && defined(Bexport)
# define EXPORT_B __declspec(dllexport)
#else
# define EXPORT_B
#endif
