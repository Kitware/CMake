#pragma once

#ifdef MUST_HAVE_DEFINE_MBCS
#  ifndef _MBCS
#    error "_MBCS is not defined, but it should be"
#  endif
#  if _MBCS != 1
#    error "_MBCS is not defined as 1, but it should be"
#  endif
#else
#  ifdef _MBCS
#    error "_MBCS is defined, but it should not be"
#  endif
#endif

#ifdef MUST_HAVE_DEFINE_SBCS
#  ifndef _SBCS
#    error "_SBCS is not defined, but it should be"
#  endif
#  if _SBCS != 1
#    error "_SBCS is not defined as 1, but it should be"
#  endif
#else
#  ifdef _SBCS
#    error "_SBCS is defined, but it should not be"
#  endif
#endif

#ifdef MUST_HAVE_DEFINE_UNICODE
#  ifndef _UNICODE
#    error "_UNICODE is not defined, but it should be"
#  endif
#  if _UNICODE != 1
#    error "_UNICODE is not defined as 1, but it should be"
#  endif
#else
#  ifdef _UNICODE
#    error "_UNICODE is defined, but it should not be"
#  endif
#endif

int FUNCTION()
{
  return 0;
}
