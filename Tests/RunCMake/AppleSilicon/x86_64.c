#ifndef __x86_64__
#  error "Not compiling as x86_64"
#endif
#ifdef __aarch64__
#  error "Incorrectly compiling as arm64"
#endif
void x86_64_arch(void)
{
}
