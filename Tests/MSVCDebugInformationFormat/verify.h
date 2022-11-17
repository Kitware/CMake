#ifdef VERIFY_Z7
#  ifndef TEST_Z7
#    error "TEST_Z7 incorrectly not defined by debug format selection"
#  endif
#else
#  ifdef TEST_Z7
#    error "TEST_Z7 incorrectly defined by non-debug format selection"
#  endif
#endif

#ifdef VERIFY_Zi
#  ifndef TEST_Zi
#    error "TEST_Zi incorrectly not defined by debug format selection"
#  endif
#else
#  ifdef TEST_Zi
#    error "TEST_Zi incorrectly defined by non-debug format selection"
#  endif
#endif

#ifdef VERIFY_ZI
#  ifndef TEST_ZI
#    error "TEST_ZI incorrectly not defined by debug format selection"
#  endif
#else
#  ifdef TEST_ZI
#    error "TEST_ZI incorrectly defined by non-debug format selection"
#  endif
#endif
