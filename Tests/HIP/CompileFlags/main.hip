#ifdef __HIP_DEVICE_COMPILE__
#  ifndef __gfx803__
#    error "Passed architecture gfx803, but got something else."
#  endif
#endif

#ifndef ALWAYS_DEFINE
#  error "ALWAYS_DEFINE not defined!"
#endif

int main()
{
}
