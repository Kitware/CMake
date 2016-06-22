struct Foo
{
  virtual int test() const = 0;
};

struct Bar : Foo
{
  int test() const override { return 0; }
};

int test(Foo const& foo)
{
  return foo.test();
}

int main()
{
  Bar const bar;
  return test(bar);
}
