
int dynamic_base_func(int);
int cuda_dynamic_host_func(int);
int file3_launch_kernel(int);

int dynamic_final_func(int x)
{
  return cuda_dynamic_host_func(dynamic_base_func(x));
}

int call_cuda_seperable_code(int x)
{
  return file3_launch_kernel(x);
}
