static void unused_function();

#ifdef __SUNPRO_C
KandR(x) int x;
{
  return x;
}
#endif

#ifdef __SUNPRO_CC
struct A
{
  virtual ~A() throw();
};
struct B : public A
{
  virtual ~B() throw(int);
};
#endif

int main(int argc, char* argv[])
{
  unsigned int unused_sign_conversion = -1;
  return 1;
}
