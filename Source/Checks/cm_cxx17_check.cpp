#include <cstdio>
#include <iterator>
#include <memory>
#include <unordered_map>

int main()
{
  int a[] = { 0, 1, 2 };
  auto ai = std::cbegin(a);

  int b[] = { 2, 1, 0 };
  auto bi = std::cend(b);

  auto ci = std::size(a);

  std::unique_ptr<int> u(new int(0));
  return *u + *ai + *(bi - 1) + (3 - static_cast<int>(ci));
}
