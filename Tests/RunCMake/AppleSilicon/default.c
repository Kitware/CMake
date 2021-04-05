#if defined(HOST_ARM64)
#  if !defined(__aarch64__)
#    error "Not compiling as host arm64"
#  endif
#elif defined(HOST_X86_64)
#  if !defined(__x86_64__)
#    error "Not compiling as host x86_64"
#  endif
#else
#  error "One of HOST_ARM64 or HOST_X86_64 must be defined."
#endif
void default_arch(void)
{
}
