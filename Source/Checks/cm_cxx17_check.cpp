#include <cstdio>
#include <iterator>
#include <memory>
#include <optional>
#include <unordered_map>

#ifdef _MSC_VER
#  include <comdef.h>
#endif

template <typename T,
          typename std::invoke_result<decltype(&T::get), T>::type = nullptr>
typename T::pointer get_ptr(T& item)
{
  return item.get();
}

int main()
{
  int a[] = { 0, 1, 2 };
  auto ai = std::cbegin(a);

  int b[] = { 2, 1, 0 };
  auto bi = std::cend(b);

  auto ci = std::size(a);

  std::unique_ptr<int> u(new int(0));

  // Intel compiler do not handle correctly 'decltype' inside 'invoke_result'
  get_ptr(u);

#ifdef _MSC_VER
  // clang-cl has problems instantiating this constructor in C++17 mode
  //  error: indirection requires pointer operand ('const _GUID' invalid)
  //      return *_IID;
  IUnknownPtr ptr{};
  IDispatchPtr disp(ptr);
#endif

  std::optional<int> oi = 0;

  return *u + *ai + *(bi - 1) + (3 - static_cast<int>(ci)) + oi.value();
}
