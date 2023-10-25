
#if DEFAULT_C23
#  if __STDC_VERSION__ <= 201710L
#    error Unexpected value for __STDC_VERSION__.
#  endif
#elif DEFAULT_C17
#  if __STDC_VERSION__ < 201710L
#    error Unexpected value for __STDC_VERSION__.
#  endif
#elif DEFAULT_C11
#  if __STDC_VERSION__ < 201112L
#    error Unexpected value for __STDC_VERSION__.
#  endif
#elif DEFAULT_C99
#  if __STDC_VERSION__ != 199901L
#    error Unexpected value for __STDC_VERSION__.
#  endif
#else
#  if !DEFAULT_C90
#    error Buildsystem error
#  endif
#  if defined(__STDC_VERSION__) &&                                            \
    !(__STDC_VERSION__ == 199409L &&                                          \
      (defined(__INTEL_COMPILER) || defined(__SUNPRO_C)))
#    error Unexpected __STDC_VERSION__ definition
#  endif
#endif

int main(void)
{
  return 0;
}
