#include <cstdio>
#include <iterator>
#include <memory>

int main()
{
  int a[] = { 0, 1, 2 };
  auto ai = std::cbegin(a);

  int b[] = { 2, 1, 0 };
  auto bi = std::cend(b);

  std::unique_ptr<int> u(new int(0));
  return *u + *ai + *(bi - 1);
}
