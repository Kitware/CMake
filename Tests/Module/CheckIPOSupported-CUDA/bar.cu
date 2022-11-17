__device__ int foo_func(int);

void __global__ bar_kernel(int x)
{
  foo_func(x);
}

int launch_kernel(int x)
{
  bar_kernel<<<1, 1>>>(x);
  return x;
}
