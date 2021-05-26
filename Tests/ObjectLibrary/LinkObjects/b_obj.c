#ifndef OBJB
#  error "OBJB is not defined, but should be"
#endif
extern int b_dep(void);
int b_obj(void)
{
  return b_dep();
}
