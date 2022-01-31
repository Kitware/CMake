#ifndef DEF_DIRECT_FROM_A
#  error "DEF_DIRECT_FROM_A incorrectly not defined"
#endif
#ifdef DEF_DIRECT_FROM_A_FOR_EXE
#  error "DEF_DIRECT_FROM_A_FOR_EXE incorrectly defined"
#endif
#ifdef DEF_DIRECT_FROM_A_OPTIONAL
#  error "DEF_DIRECT_FROM_A_OPTIONAL incorrectly defined"
#endif

extern void a_always(void);

void static_A_public(void)
{
  a_always();
}
