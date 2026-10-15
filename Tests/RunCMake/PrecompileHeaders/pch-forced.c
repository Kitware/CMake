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

int forced(void);

int forced(void)
{
  return FORCED2_INCLUDED - FORCED_INCLUDED;
}
