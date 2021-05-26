#ifdef OBJA
#  error "OBJA is defined, but should not be"
#endif
#ifndef OBJB
#  error "OBJB is not defined, but should be"
#endif
#ifdef OBJC
#  error "OBJC is defined, but should not be"
#endif
#ifndef OBJD
#  error "OBJD is not defined, but should be"
#endif
#ifdef OBJE
#  error "OBJE is defined, but should not be"
#endif
extern int a_obj(void);
extern int b_obj(void);
extern int c_obj(void);
extern int d_obj(void);
extern int e_lib(void);
int main(void)
{
  return a_obj() + b_obj() + c_obj() + d_obj() + e_lib();
}
