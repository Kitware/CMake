#ifdef __CUDA_ARCH__
#  if __CUDA_ARCH__ != 500
#    error "Passed architecture 50, but got something else."
#  endif
#endif

#ifndef ALWAYS_DEFINE
#  error "ALWAYS_DEFINE not defined!"
#endif

int main()
{
}
