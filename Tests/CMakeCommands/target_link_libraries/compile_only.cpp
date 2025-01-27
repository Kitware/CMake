
#ifndef USE_EXAMPLE
#  error "Missing propagated define"
#endif

#ifdef HAVE_FUNCTION
int non_duplicate_function()
{
  return 42;
}
#endif

// Solaris needs non-empty content so ensure
// we have at least one symbol
int Solaris_requires_a_symbol_here = 0;
