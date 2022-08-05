#ifdef VERIFY_DLL
#  ifndef _DLL
#    error "_DLL not defined by DLL runtime library selection"
#  endif
#  ifndef __SW_BR
#    error "__SW_BR not defined by DLL runtime library selection"
#  endif
#else
#  ifdef _DLL
#    error "_DLL defined by non-DLL runtime library selection"
#  endif
#  ifdef __SW_BR
#    error "__SW_BR defined by non-DLL runtime library selection"
#  endif
#endif

#ifdef VERIFY_MT
#  ifndef _MT
#    error "_MT not defined by multi-threaded runtime library selection"
#  endif
#  ifndef __SW_BM
#    error "__SW_BM not defined by multi-threaded runtime library selection"
#  endif
#else
#  ifdef _MT
#    error "_MT defined by single-threaded runtime library selection"
#  endif
#  ifdef __SW_BM
#    error "__SW_BM defined by single-threaded runtime library selection"
#  endif
#endif
