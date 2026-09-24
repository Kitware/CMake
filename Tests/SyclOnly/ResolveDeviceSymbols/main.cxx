bool device_available();
int launch_kernel();
#ifdef TEST_MIXED_KERNEL
int launch_mixed_kernel();
#endif

int main()
{
  if (!device_available()) {
    return 77;
  }
  if (launch_kernel() != 42) {
    return 1;
  }
#ifdef TEST_MIXED_KERNEL
  if (launch_mixed_kernel() != 7) {
    return 1;
  }
#endif
  return 0;
}
