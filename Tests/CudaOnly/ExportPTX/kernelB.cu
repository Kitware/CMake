
#ifndef CUDA_PTX_COMPILATION
#  error "CUDA_PTX_COMPILATION define not provided"
#endif

__global__ void kernelB(float* r, float* x, float* y, float* z, int size)
{
  for (int i = threadIdx.x; i < size; i += blockDim.x) {
    r[i] = x[i] * y[i] + z[i];
  }
}
