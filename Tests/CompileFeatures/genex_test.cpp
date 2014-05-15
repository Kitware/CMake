
#if !HAVE_OVERRIDE_CONTROL
#error "Expect override control feature"
#else

struct A
{
  virtual int getA() { return 7; }
};

struct B final : A
{
  int getA() override { return 42; }
};

#endif

int main()
{

}
