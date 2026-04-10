#ifdef __POCC__
#  pragma warn(disable : 2801) /* Store to a non-writable location. */
#endif

int main(int argc, char const* argv[])
{
#ifndef __clang_analyzer__ /* Suppress clang-analyzer warnings */
  /* Construct an invalid address that cannot be predicted by the
     compiler/optimizer, and that is not NULL (which is undefined
     behavior to dereference).  */
  int volatile* invalidAddress = 0;
  invalidAddress += argc ? 1 : 2;
  (void)argv;
  /* Write to the invalid address to cause SIGSEGV or similar.  */
  *invalidAddress = 0;
#endif
  return 0;
}
