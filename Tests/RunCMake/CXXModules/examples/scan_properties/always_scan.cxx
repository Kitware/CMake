#if SCANNING_CONTROL
#  ifndef CMAKE_SCANNED_THIS_SOURCE
#    error "This file should have been scanned"
#  endif
#endif

import M;

int scanned_file()
{
  return from_module();
}
