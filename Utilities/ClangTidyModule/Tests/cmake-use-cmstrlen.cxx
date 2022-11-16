#include <cstring>

template <size_t N>
constexpr size_t cmStrLen(const char (&/*str*/)[N])
{
  return N - 1;
}

namespace ns1 {
using std::strlen;
}

namespace ns2 {
std::size_t strlen(const char* str)
{
  return std::strlen(str);
}
}

int main()
{
  // String variable used for calling strlen() on a variable
  auto s0 = "howdy";

  // Correction needed
  (void)strlen("Hello");
  (void)::strlen("Goodbye");
  (void)std::strlen("Hola");
  (void)ns1::strlen("Bonjour");
  (void)(sizeof("Hallo") - 1);
  (void)(4 + sizeof("Hallo") - 1);
  (void)(sizeof "Hallo" - 1);
  (void)(4 + sizeof "Hallo" - 1);

  // No correction needed
  (void)ns2::strlen("Salve");
  (void)cmStrLen("Konnichiwa");
  (void)strlen(s0);
  (void)(sizeof("Hallo") - 2);

  return 0;
}
