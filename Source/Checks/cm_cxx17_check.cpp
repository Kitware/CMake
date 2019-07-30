#include <cstdio>
#include <iterator>
#include <memory>
#include <unordered_map>

#ifdef _MSC_VER
#  include <comdef.h>
#endif

int main()
{
  int a[] = { 0, 1, 2 };
  auto ai = std::cbegin(a);

  int b[] = { 2, 1, 0 };
  auto bi = std::cend(b);

  auto ci = std::size(a);

  std::unique_ptr<int> u(new int(0));

#ifdef _MSC_VER
  // clang-cl has problems instantiating this constructor in C++17 mode
  //  error: indirection requires pointer operand ('const _GUID' invalid)
  //      return *_IID;
  IUnknownPtr ptr{};
  IDispatchPtr disp(ptr);
#endif

  return *u + *ai + *(bi - 1) + (3 - static_cast<int>(ci));
}
