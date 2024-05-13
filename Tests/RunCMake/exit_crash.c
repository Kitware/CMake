int main(int argc, const char* argv[])
{
#ifndef __clang_analyzer__ /* Suppress clang-analyzer warnings */
  /* Construct an invalid address that cannot be predicted by the
     compiler/optimizer, and that is not NULL (which is undefined
     behavior to dereference).  */
  volatile int* invalidAddress = 0;
  invalidAddress += argc ? 1 : 2;
  (void)argv;
  /* Write to the invalid address to cause SIGSEGV or similar.  */
  *invalidAddress = 0;
#endif
  return 0;
}
