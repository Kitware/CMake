#include <string>
#include <utility>

template <typename... Args>
std::string cmStrCat(Args&&... args)
{
  return "";
}

std::string a = "This is a string variable";
std::string b = " and this is a string variable";
std::string concat;

// Correction needed
void test1()
{
  concat = a + b;
  concat = a + " and this is a string literal";
  concat = a + 'O';
  concat = "This is a string literal" + b;
  concat = 'O' + a;
  concat = a + " and this is a string literal" + 'O' + b;

  concat += b;
  concat += " and this is a string literal";
  concat += 'o';
  concat += b + " and this is a string literal " + 'o' + b;

  std::pair<std::string, std::string> p;
  concat = p.first + p.second;
}

// No correction needed
void test2()
{
  a = b;
  a = "This is a string literal";
  a = 'X';
  cmStrCat(a, b);
}
