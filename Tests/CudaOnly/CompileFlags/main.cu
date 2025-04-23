#ifndef EXPECT_CUDA_ARCH
#  error "EXPECT_CUDA_ARCH not defined!"
#endif
#ifdef __CUDA_ARCH__
#  if __CUDA_ARCH__ != (EXPECT_CUDA_ARCH * 10)
#    error "__CUDA_ARCH__ does not match CUDA_ARCHITECTURES"
#  endif
#endif

#ifndef ALWAYS_DEFINE
#  error "ALWAYS_DEFINE not defined!"
#endif

int main()
{
}
