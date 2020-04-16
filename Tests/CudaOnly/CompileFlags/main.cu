#ifdef __CUDA_ARCH__
#  if __CUDA_ARCH__ != 500
#    error "Passed architecture 50, but got something else."
#  endif
#endif

// Check HOST_DEFINE only for nvcc
#ifndef __CUDA__
#  ifndef HOST_DEFINE
#    error "HOST_DEFINE not defined!"
#  endif
#endif

int main()
{
}
