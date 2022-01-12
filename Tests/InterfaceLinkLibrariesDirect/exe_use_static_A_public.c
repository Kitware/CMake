#ifndef DEF_DIRECT_FROM_A
#  error "DEF_DIRECT_FROM_A incorrectly not defined"
#endif
#ifndef DEF_DIRECT_FROM_A_FOR_EXE
#  error "DEF_DIRECT_FROM_A_FOR_EXE incorrectly not defined"
#endif
#ifndef DEF_DIRECT_FROM_A_OPTIONAL
#  error "DEF_DIRECT_FROM_A_OPTIONAL incorrectly not defined"
#endif

extern void static_A_public(void);
extern void direct_from_A(void);
extern void direct_from_A_for_exe(void);
extern void direct_from_A_optional(void);

int main(void)
{
  static_A_public();
  direct_from_A();
  direct_from_A_for_exe();
  direct_from_A_optional();
  return 0;
}
