extern __device__ int file2_func(int);
void __global__ kernel(int x)
{
  file2_func(x);
}

int launch_kernel(int x)
{
  kernel<<<1, 1>>>(x);
  return x;
}
