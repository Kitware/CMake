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

int join();

int main(int argc, char** argv)
{
  return join();
}
