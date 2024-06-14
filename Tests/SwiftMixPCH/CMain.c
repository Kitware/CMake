#ifndef PCH_VALUE
#  error "PCH_VALUE not defined"
#endif

int main(void)
{
  const int value = PCH_VALUE;
  return value == 42 ? 0 : 1;
}
