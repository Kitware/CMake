#ifndef FLAGS_ALL_LANGUAGES
#  error FLAGS_ALL_LANGUAGES is not defined in C++ sources
#endif
#ifdef FLAGS_ONLY_IN_C
#  error FLAGS_ONLY_IN_C is defined in C++ sources
#endif
#ifndef FLAGS_ONLY_IN_CXX
#  error FLAGS_ONLY_IN_CXX is not defined in C++ sources
#endif
