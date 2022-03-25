#ifdef EXPECT_FOO_LINK_ONLY
#  ifndef FOO_LINK_ONLY
#    error "FOO_LINK_ONLY incorrectly not defined"
#  endif
#else
#  ifdef FOO_LINK_ONLY
#    error "FOO_LINK_ONLY incorrectly defined"
#  endif
#endif

extern int foo_link_only(void);

int main(void)
{
  return foo_link_only();
}
