#ifndef OVERRIDE1
#  error OVERRIDE1 is not defined
#endif
#if OVERRIDE1 != 2
#  error OVERRIDE1 has the wrong value
#endif

#ifndef OVERRIDE2
#  error OVERRIDE2 is not defined
#endif
#if OVERRIDE2 != 0
#  error OVERRIDE2 has the wrong value
#endif

#ifdef ONLY_IN_C
#  error ONLY_IN_C is defined in C++ sources
#endif
#ifndef ONLY_IN_CXX
#  error ONLY_IN_CXX is not defined in C++ sources
#endif

#ifndef NOVALUE
#  error NOVALUE is not defined
#endif
#if !defined(__BORLANDC__)
#  if !NOVALUE
#    error NOVALUE evaluated as a Boolean is not true
#  endif
#endif

#ifndef EMPTYVALUE
#  error EMPTYVALUE is not defined
#endif

#if 3 - EMPTYVALUE - 3 != 6
#  error EMPTYVALUE is not empty
#endif
