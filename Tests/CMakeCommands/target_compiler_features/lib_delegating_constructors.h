
#include <cstring>

class Foo
{
public:
  Foo(int i);

  Foo(const char *a)
    : Foo(strlen(a))
  {

  }

private:
  int m_i;
};
