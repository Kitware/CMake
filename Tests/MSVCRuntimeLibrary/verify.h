#ifdef VERIFY_DEBUG
#  ifndef _DEBUG
#    error "_DEBUG not defined by debug runtime library selection"
#  endif
#else
#  ifdef _DEBUG
#    error "_DEBUG defined by non-debug runtime library selection"
#  endif
#endif

#ifdef VERIFY_DLL
#  ifndef _DLL
#    error "_DLL not defined by DLL runtime library selection"
#  endif
#else
#  ifdef _DLL
#    error "_DLL defined by non-DLL runtime library selection"
#  endif
#endif

#ifdef VERIFY_MT
#  ifndef _MT
#    error "_MT not defined by multi-threaded runtime library selection"
#  endif
#else
#  ifdef _MT
#    error "_MT defined by single-threaded runtime library selection"
#  endif
#endif
