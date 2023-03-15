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
  (void)cmStrLen("Hello");
  (void)cmStrLen("Goodbye");
  (void)cmStrLen("Hola");
  (void)cmStrLen("Bonjour");
  (void)(cmStrLen("Hallo"));
  (void)(4 + cmStrLen("Hallo"));
  (void)(cmStrLen("Hallo"));
  (void)(4 + cmStrLen("Hallo"));

  // No correction needed
  (void)ns2::strlen("Salve");
  (void)cmStrLen("Konnichiwa");
  (void)strlen(s0);
  (void)(sizeof("Hallo") - 2);

  return 0;
}
