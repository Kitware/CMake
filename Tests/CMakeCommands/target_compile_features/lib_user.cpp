
#include "lib_delegating_constructors.h"

class Bar
{
  Bar(int i)
    :m_i(i)
  {

  }

  Bar(const char *a)
    : Bar(strlen(a))
  {

  }

private:
  int m_i;
};

int main(int argc, char **argv)
{
  Foo f("hello");
  Foo b("world");
  return 0;
}
