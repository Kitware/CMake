
int foo(int* restrict a, int* restrict b)
{
  (void)a;
  (void)b;
  return 0;
}

int main(void)
{
  return 0;
}
