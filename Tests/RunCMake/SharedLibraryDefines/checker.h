#pragma once

#ifdef MUST_HAVE_DEFINE
#  ifndef _WINDLL
#    error "_WINDLL is not defined, but it should be"
#  endif
#  if _WINDLL != 1
#    error "_WINDLL is not defined as 1, but it should be"
#  endif
#else
#  ifdef _WINDLL
#    error "_WINDLL is defined, but it should not be"
#  endif
#endif

int FUNCTION(void)
{
  return 0;
}
