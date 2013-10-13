
#include <cstring>

class Foo
{
  Foo(int i)
    :m_i(i)
  {

  }

  Foo(const char *a)
    : Foo(strlen(a))
  {

  }

private:
  int m_i;
};

int main(int, char **)
{
  return 0;
}
