int launch_kernel();
int sycl_value();
int mixed_value()
{
  return launch_kernel() + sycl_value();
}
