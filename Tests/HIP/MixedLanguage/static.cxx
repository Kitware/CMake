
#include <type_traits>

int static_cxx_func(int x)
{
  return x * x + std::integral_constant<int, 14>::value;
}
