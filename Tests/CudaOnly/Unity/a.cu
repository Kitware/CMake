
__device__ int function(int a, int b);

__global__ void kernel()
{
  function(2, 3);
}

void test_unity_functions()
{
  kernel<<<1, 1, 1>>>();
}
