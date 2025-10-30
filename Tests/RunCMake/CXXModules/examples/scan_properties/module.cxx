export module M;

#if SCANNING_CONTROL
#  ifndef CMAKE_SCANNED_THIS_SOURCE
#    error "This file should have been scanned"
#  endif
#endif

export int from_module()
{
  return 0;
}
