#ifndef PCH_VALUE
#  error "PCH_VALUE not defined"
#endif

int main(void)
{
  int const value = PCH_VALUE;
  return value == 42 ? 0 : 1;
}
