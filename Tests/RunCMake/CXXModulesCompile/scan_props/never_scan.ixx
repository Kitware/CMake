#if SCANNING_CONTROL
#  ifdef CMAKE_SCANNED_THIS_SOURCE
#    error "This file should not have been scanned"
#  endif
#endif

int never_scan()
{
  return 0;
}
