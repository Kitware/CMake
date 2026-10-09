__global__ void object_library_kernel()
{
}

void object_library_launch()
{
  object_library_kernel<<<1, 1>>>();
}
