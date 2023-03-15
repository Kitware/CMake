#if SCANNING_CONTROL
#  if SCAN_AT_TARGET_LEVEL
#    ifndef CMAKE_SCANNED_THIS_SOURCE
#      error "This file should have been scanned"
#    endif
#  else
#    ifdef CMAKE_SCANNED_THIS_SOURCE
#      error "This file should not have been scanned"
#    endif
#  endif
#endif

int scanned_file();
int never_scan();

int join()
{
  return scanned_file() + never_scan();
}
