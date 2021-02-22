#ifdef CXX_PCH
#  error "CXX PCH included in C source."
#endif
int no_pch(void)
{
  return 0;
}
