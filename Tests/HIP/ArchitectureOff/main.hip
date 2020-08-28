#ifdef __HIP_DEVICE_COMPILE__
#  ifndef __gfx908__
#    error "Passed architecture gfx908, but got something else."
#  endif
#endif

int main()
{
}
