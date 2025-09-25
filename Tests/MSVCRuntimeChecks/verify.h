/* Some compilers ignore the -RTC flags even if specified.  */
#if (defined(_MSC_VER) && _MSC_VER <= 1310) || defined(__clang__) ||          \
  defined(__INTEL_LLVM_COMPILER) || defined(__INTEL_COMPILER)
#  define NO__MSVC_RUNTIME_CHECKS
#endif

#ifdef VERIFY_RTCc
#  ifndef TEST_RTCc
#    error "TEST_RTCc incorrectly not defined by enabled runtime check"
#  endif
#  if !defined(__MSVC_RUNTIME_CHECKS) && !defined(NO__MSVC_RUNTIME_CHECKS)
#    error                                                                    \
      "__MSVC_RUNTIME_CHECKS incorrectly not defined by enabled RTCc runtime check"
#  endif
#else
#  ifdef TEST_RTCc
#    error "TEST_RTCc incorrectly defined by disabled runtime check"
#  endif
#endif

#ifdef VERIFY_RTCs
#  ifndef TEST_RTCs
#    error "TEST_RTCs incorrectly not defined by enabled runtime check"
#  endif
#  if !defined(__MSVC_RUNTIME_CHECKS) && !defined(NO__MSVC_RUNTIME_CHECKS)
#    error                                                                    \
      "__MSVC_RUNTIME_CHECKS incorrectly not defined by enabled RTCs runtime check"
#  endif
#else
#  ifdef TEST_RTCs
#    error "TEST_RTCs incorrectly defined by disabled runtime check"
#  endif
#endif

#ifdef VERIFY_RTCu
#  ifndef TEST_RTCu
#    error "TEST_RTCu incorrectly not defined by enabled runtime check"
#  endif
#  if !defined(__MSVC_RUNTIME_CHECKS) && !defined(NO__MSVC_RUNTIME_CHECKS)
#    error                                                                    \
      "__MSVC_RUNTIME_CHECKS incorrectly not defined by enabled RTCu runtime check"
#  endif
#else
#  ifdef TEST_RTCu
#    error "TEST_RTCu incorrectly defined by disabled runtime check"
#  endif
#endif

#ifdef VERIFY_RTCsu
#  ifndef TEST_RTCsu
#    error "TEST_RTCsu incorrectly not defined by enabled runtime check"
#  endif
#  if !defined(__MSVC_RUNTIME_CHECKS) && !defined(NO__MSVC_RUNTIME_CHECKS)
#    error                                                                    \
      "__MSVC_RUNTIME_CHECKS incorrectly not defined by enabled RTCsu runtime check"
#  endif
#else
#  ifdef TEST_RTCsu
#    error "TEST_RTCsu incorrectly defined by disabled runtime check"
#  endif
#endif
