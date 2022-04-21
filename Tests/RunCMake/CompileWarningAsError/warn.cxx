static void unused_function();

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

int main(int unused_argument, char* [])
{
  return 1;
}
