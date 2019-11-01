
#include <type_traits>

int main(int argc, char** argv)
{
  // Verify that we have at least c++11
  using returnv = std::integral_constant<int, 0>;
  return returnv::value;
}
