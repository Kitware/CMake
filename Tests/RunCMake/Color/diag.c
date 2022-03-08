#ifdef EXPECT_COLOR
#  if EXPECT_COLOR
#    ifndef COLOR_ON
#      error "COLOR_ON incorrectly not defined"
#    endif
#    ifdef COLOR_OFF
#      error "COLOR_OFF incorrectly defined"
#    endif
#  else
#    ifdef COLOR_ON
#      error "COLOR_ON incorrectly defined"
#    endif
#    ifndef COLOR_OFF
#      error "COLOR_OFF incorrectly not defined"
#    endif
#  endif
#else
#  ifdef COLOR_ON
#    error "COLOR_ON incorrectly defined"
#  endif
#  ifdef COLOR_OFF
#    error "COLOR_OFF incorrectly defined"
#  endif
#endif

void diag(void)
{
}
