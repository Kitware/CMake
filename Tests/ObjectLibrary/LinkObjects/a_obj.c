#ifndef OBJA
#  error "OBJA is not defined, but should be"
#endif
extern int a_dep(void);
int a_obj(void)
{
  return a_dep();
}
