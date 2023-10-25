#ifndef BUILD_DEBUG
#  error BUILD_DEBUG is undefined
#endif
#ifndef GLOBAL_DEBUG
#  error GLOBAL_DEBUG is undefined
#endif
#ifndef TARGET_DEBUG
#  error TARGET_DEBUG is undefined
#endif

#if GLOBAL_DEBUG != BUILD_DEBUG
#  error GLOBAL_DEBUG does not match BUILD_DEBUG
#endif
#if TARGET_DEBUG != BUILD_DEBUG
#  error TARGET_DEBUG does not match BUILD_DEBUG
#endif

void some_symbol(void)
{
}
