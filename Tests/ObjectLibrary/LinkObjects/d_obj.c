#ifndef OBJD
#  error "OBJD is not defined, but should be"
#endif
extern int d_dep(void);
int d_obj(void)
{
  return d_dep();
}
