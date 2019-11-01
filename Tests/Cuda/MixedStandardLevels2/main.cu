
#if __cplusplus < 201103L && !defined(_MSC_VER)
#  error "invalid standard value"
#endif
#include <type_traits>

int main(int argc, char** argv)
{
  using returnv = std::integral_constant<int, 0>;
  return returnv::value;
}
