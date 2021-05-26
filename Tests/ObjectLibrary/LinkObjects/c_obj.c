#ifndef OBJC
#  error "OBJC is not defined, but should be"
#endif
extern int c_dep(void);
int c_obj(void)
{
  return c_dep();
}
