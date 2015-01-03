
#if !HAVE_OVERRIDE_CONTROL
#if EXPECT_OVERRIDE_CONTROL
#error "Expect override control feature"
#endif
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

#if !HAVE_NULLPTR
#error "Expect nullptr feature"
#else

const char* getString()
{
  return nullptr;
}

#endif

int main()
{

}
