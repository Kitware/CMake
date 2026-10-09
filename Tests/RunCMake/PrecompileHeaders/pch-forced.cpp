#ifndef FORCED_INCLUDED
#  error "forced.h was not force-included"
#endif

#ifndef FORCED2_INCLUDED
#  error "forced2.h was not force-included"
#endif

#ifndef FORCED3_INCLUDED
#  error "forced3.h was not force-included"
#endif

#ifndef FORCED4_INCLUDED
#  error "forced4.h was not force-included"
#endif

#ifndef FORCED5_INCLUDED
#  error "forced5.h was not force-included"
#endif

#ifdef HAVE_PCH_SUPPORT
#  ifndef PCH_INCLUDED
#    error "pch-forced.h was not included by the precompiled header"
#  endif
#else
#  include "pch-forced.h"
#endif

extern "C" int forced(void);

int main()
{
  return (PCH_INCLUDED - FORCED_INCLUDED) + forced();
}
