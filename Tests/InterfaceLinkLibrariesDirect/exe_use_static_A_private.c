#ifdef DEF_DIRECT_FROM_A
#  error "DEF_DIRECT_FROM_A incorrectly defined"
#endif
#ifdef DEF_DIRECT_FROM_A_FOR_EXE
#  error "DEF_DIRECT_FROM_A_FOR_EXE incorrectly defined"
#endif
#ifdef DEF_DIRECT_FROM_A_OPTIONAL
#  error "DEF_DIRECT_FROM_A_OPTIONAL incorrectly defined"
#endif

extern void static_A_private(void);
extern void direct_from_A(void);
extern void direct_from_A_for_exe(void);
extern void not_direct_from_A_optional(void);

int main(void)
{
  static_A_private();
  direct_from_A();
  direct_from_A_for_exe();
  not_direct_from_A_optional();
  return 0;
}
