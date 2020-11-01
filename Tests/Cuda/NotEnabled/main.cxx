
#include <type_traits>

int main(int argc, char** argv)
{
  // make sure we have c++11 enabled
  using returnv = std::integral_constant<int, 0>;
  return returnv::value;
}
