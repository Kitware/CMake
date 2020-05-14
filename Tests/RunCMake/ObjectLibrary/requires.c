#ifdef REQUIRED
int required(void)
{
  return 0;
}
#else
#  error "REQUIRED not defined"
#endif
