#include "cxx_std.h"

template <long l>
struct Outputter;

#if DEFAULT_CXX26
#  if CXX_STD <= CXX_STD_23
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX23
#  if CXX_STD <= CXX_STD_20
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX20
#  if CXX_STD <= CXX_STD_17
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX17
#  if CXX_STD <= CXX_STD_14
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX14
#  if CXX_STD <= CXX_STD_11
Outputter<CXX_STD> o;
#  endif
#elif DEFAULT_CXX11
#  if CXX_STD != CXX_STD_11
Outputter<CXX_STD> o;
#  endif
#else
#  if !DEFAULT_CXX98
#    error Buildsystem error
#  endif
#  if CXX_STD != CXX_STD_98 && CXX_STD != 1 &&                                \
    !defined(__GXX_EXPERIMENTAL_CXX0X__)
Outputter<CXX_STD> o;
#  endif
#endif

int main()
{
  return 0;
}
