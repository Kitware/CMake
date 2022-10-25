#if !defined(FOO)
#  error "FOO not defined"
#endif
#if BAR != 3
#  error "FOO not defined to 3"
#endif
#if CCOND != 2
#  error "CCOND not defined to 2"
#endif
#if defined(SWIFTCOND)
#  error "SWIFTCOND defined"
#endif

extern int ObjCMain(void);
int main(void)
{
  return ObjCMain();
}
