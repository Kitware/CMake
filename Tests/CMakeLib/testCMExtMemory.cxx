#include <iostream>
#include <memory>

#include <cmext/memory>

namespace {
class Base
{
public:
  virtual ~Base() = default;
};

class Derived : public Base
{
public:
  ~Derived() override = default;

  void method() {}
};

template <typename T>
class Wrapper
{
public:
  Wrapper(T* v)
    : value(v)
  {
  }
  ~Wrapper() { delete this->value; }

  T* get() const { return this->value; }

private:
  T* value;
};

bool testReferenceCast()
{
  std::cout << "testReferenceCast()" << std::endl;

  std::unique_ptr<Base> u(new Derived);
  cm::static_reference_cast<Derived>(u).method();
  cm::dynamic_reference_cast<Derived>(u).method();

  std::shared_ptr<Base> s(new Derived);
  cm::static_reference_cast<Derived>(s).method();
  cm::dynamic_reference_cast<Derived>(s).method();

  // can also be used with custom wrappers
  Wrapper<Base> w(new Derived);
  cm::static_reference_cast<Derived>(w).method();
  cm::dynamic_reference_cast<Derived>(w).method();

  return true;
}
}

int testCMExtMemory(int /*unused*/, char* /*unused*/[])
{
  if (!testReferenceCast()) {
    return 1;
  }

  return 0;
}
