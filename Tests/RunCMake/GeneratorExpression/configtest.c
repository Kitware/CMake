#ifdef EXPECT_DEBUG
#  ifndef DEBUG
#    error DEBUG should be defined
#  endif
#else
#  ifdef DEBUG
#    error DEBUG should not be defined
#  endif
#endif

#ifdef EXPECT_RELEASE
#  ifndef RELEASE
#    error RELEASE should be defined
#  endif
#else
#  ifdef RELEASE
#    error RELEASE should not be defined
#  endif
#endif

#ifdef EXPECT_TEST
#  ifndef TEST
#    error TEST should be defined
#  endif
#else
#  ifdef TEST
#    error TEST should not be defined
#  endif
#endif

int main(void)
{
  return 0;
}
