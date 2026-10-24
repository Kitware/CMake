#include <compare>
#include <concepts>
#include <cstdio>
#include <memory>
#include <span>
#include <string>
#include <string_view>

#ifdef _MSC_VER
#  include <comdef.h>
#endif

template <std::integral T>
T twice(T t)
{
  return t * 2;
}

struct Point
{
  int x = 0;
  int y = 0;
  auto operator<=>(Point const&) const = default;
};

int main()
{
  int a[] = { 0, 1, 2 };
  std::span<int const> s(a);

  Point p{ .x = 1, .y = 2 };
  Point q{ .x = 1, .y = 2 };

  std::string str = "cmake";
  std::string_view sv = str;

  std::unique_ptr<int> u(new int(0));

#ifdef _MSC_VER
  // see this check under C++17
  IUnknownPtr ptr{};
  IDispatchPtr disp(ptr);
#endif

  return *u + s[0] + twice(0) + (p == q ? 0 : 1) +
    (sv.starts_with("cm") && str.ends_with("ke") ? 0 : 1);
}
