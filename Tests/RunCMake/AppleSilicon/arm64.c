#ifndef __aarch64__
#  error "Not compiling as arm64"
#endif
#ifdef __x86_64__
#  error "Incorrectly compiling as x86_64"
#endif
void arm64_arch(void)
{
}
