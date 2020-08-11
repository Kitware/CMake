#ifdef __CUDA_ARCH__
#  if __CUDA_ARCH__ != 520
#    error "Passed architecture 52, but got something else."
#  endif
#endif

int main()
{
}
