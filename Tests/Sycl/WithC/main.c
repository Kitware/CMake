int sycl_value(void);
int main(void)
{
  return sycl_value() == 42 ? 0 : 1;
}
