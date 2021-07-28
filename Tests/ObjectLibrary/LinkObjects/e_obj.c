#ifndef OBJE
#  error "OBJE is not defined, but should be"
#endif
extern int e_dep(void);
int e_obj(void)
{
  return e_dep();
}
